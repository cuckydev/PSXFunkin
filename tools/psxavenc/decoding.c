/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder frontend

Copyright (c) 2019, 2020 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

This tool has been modified so that libpsxav isn't an external library
*/

#include "common.h"

static void poll_av_packet(settings_t *settings, AVPacket *packet);

int decode_audio_frame(AVCodecContext *codec, AVFrame *frame, int *frame_size, AVPacket *packet) {
	int ret;

	if (packet != NULL) {
		ret = avcodec_send_packet(codec, packet);
		if (ret != 0) {
			return 0;
		}
	}

	ret = avcodec_receive_frame(codec, frame);
	if (ret >= 0) {
		*frame_size = ret;
		return 1;
	} else {
		return ret == AVERROR(EAGAIN) ? 1 : 0;
	}
}

int decode_video_frame(AVCodecContext *codec, AVFrame *frame, int *frame_size, AVPacket *packet) {
	int ret;

	if (packet != NULL) {
		ret = avcodec_send_packet(codec, packet);
		if (ret != 0) {
			return 0;
		}
	}

	ret = avcodec_receive_frame(codec, frame);
	if (ret >= 0) {
		*frame_size = ret;
		return 1;
	} else {
		return ret == AVERROR(EAGAIN) ? 1 : 0;
	}
}

bool open_av_data(const char *filename, settings_t *settings)
{
	AVPacket packet;

	av_decoder_state_t* av = &(settings->decoder_state_av);
	av->video_next_pts = 0.0;
	av->frame = NULL;
	av->video_frame_src_size = 0;
	av->video_frame_dst_size = 0;
	av->audio_stream_index = -1;
	av->video_stream_index = -1;
	av->format = NULL;
	av->audio_stream = NULL;
	av->video_stream = NULL;
	av->audio_codec_context = NULL;
	av->video_codec_context = NULL;
	av->audio_codec = NULL;
	av->video_codec = NULL;
	av->resampler = NULL;
	av->scaler = NULL;

	av->format = avformat_alloc_context();
	if (avformat_open_input(&(av->format), filename, NULL, NULL)) {
		return false;
	}
	if (avformat_find_stream_info(av->format, NULL) < 0) {
		return false;
	}

	for (int i = 0; i < av->format->nb_streams; i++) {
		if (av->format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (av->audio_stream_index >= 0) {
				fprintf(stderr, "open_av_data: found multiple audio tracks?\n");
				return false;
			}
			av->audio_stream_index = i;
		}
	}
	if (av->audio_stream_index == -1) {
		return false;
	}

	for (int i = 0; i < av->format->nb_streams; i++) {
		if (av->format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			if (av->video_stream_index >= 0) {
				fprintf(stderr, "open_av_data: found multiple video tracks?\n");
				return false;
			}
			av->video_stream_index = i;
		}
	}

	av->audio_stream = av->format->streams[av->audio_stream_index];
	av->video_stream = (av->video_stream_index != -1 ? av->format->streams[av->video_stream_index] : NULL);
	av->audio_codec = avcodec_find_decoder(av->audio_stream->codecpar->codec_id);
	av->audio_codec_context = avcodec_alloc_context3(av->audio_codec);
	if (av->audio_codec_context == NULL) {
		return false;
	}
	if (avcodec_parameters_to_context(av->audio_codec_context, av->audio_stream->codecpar) < 0) {
		return false;
	}
	if (avcodec_open2(av->audio_codec_context, av->audio_codec, NULL) < 0) {
		return false;
	}

	av->resampler = swr_alloc();
	av_opt_set_int(av->resampler, "in_channel_count", av->audio_codec_context->channels, 0);
	av_opt_set_int(av->resampler, "in_channel_layout", av->audio_codec_context->channel_layout, 0);
	av_opt_set_int(av->resampler, "in_sample_rate", av->audio_codec_context->sample_rate, 0);
	av_opt_set_sample_fmt(av->resampler, "in_sample_fmt", av->audio_codec_context->sample_fmt, 0);

	av->sample_count_mul = settings->stereo ? 2 : 1;
	av_opt_set_int(av->resampler, "out_channel_count", settings->stereo ? 2 : 1, 0);
	av_opt_set_int(av->resampler, "out_channel_layout", settings->stereo ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO, 0);
	av_opt_set_int(av->resampler, "out_sample_rate", settings->frequency, 0);
	av_opt_set_sample_fmt(av->resampler, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

	if (swr_init(av->resampler) < 0) {
		return false;
	}

	if (av->video_stream != NULL) {
		av->video_codec = avcodec_find_decoder(av->video_stream->codecpar->codec_id);
		av->video_codec_context = avcodec_alloc_context3(av->video_codec);
		if(av->video_codec_context == NULL) {
			return false;
		}
		if (avcodec_parameters_to_context(av->video_codec_context, av->video_stream->codecpar) < 0) {
			return false;
		}
		if (avcodec_open2(av->video_codec_context, av->video_codec, NULL) < 0) {
			return false;
		}

		av->scaler = sws_getContext(
			av->video_codec_context->width,
			av->video_codec_context->height,
			av->video_codec_context->pix_fmt,
			settings->video_width,
			settings->video_height,
			AV_PIX_FMT_RGBA,
			SWS_BICUBIC,
			NULL,
			NULL,
			NULL);
		
		av->video_frame_src_size = 4*av->video_codec_context->width*av->video_codec_context->height;
		av->video_frame_dst_size = 4*settings->video_width*settings->video_height;
	}

	av_init_packet(&packet);
	av->frame = av_frame_alloc();
	if (av->frame == NULL) {
		return false;
	}

	settings->audio_samples = NULL;
	settings->audio_sample_count = 0;
	settings->video_frames = NULL;
	settings->video_frame_count = 0;

	return true;
}

static void poll_av_packet_audio(settings_t *settings, AVPacket *packet)
{
	av_decoder_state_t* av = &(settings->decoder_state_av);

	int frame_size, frame_sample_count;
	uint8_t *buffer[1];

	if (decode_audio_frame(av->audio_codec_context, av->frame, &frame_size, packet)) {
		size_t buffer_size = sizeof(int16_t) * av->sample_count_mul * swr_get_out_samples(av->resampler, av->frame->nb_samples);
		buffer[0] = malloc(buffer_size);
		memset(buffer[0], 0, buffer_size);
		frame_sample_count = swr_convert(av->resampler, buffer, av->frame->nb_samples, (const uint8_t**)av->frame->data, av->frame->nb_samples);
		settings->audio_samples = realloc(settings->audio_samples, (settings->audio_sample_count + ((frame_sample_count + 4032) * av->sample_count_mul)) * sizeof(int16_t));
		memmove(&(settings->audio_samples[settings->audio_sample_count]), buffer[0], sizeof(int16_t) * frame_sample_count * av->sample_count_mul);
		settings->audio_sample_count += frame_sample_count * av->sample_count_mul;
		free(buffer[0]);
	}
}

static void poll_av_packet_video(settings_t *settings, AVPacket *packet)
{
	av_decoder_state_t* av = &(settings->decoder_state_av);

	int frame_size;

	if (decode_video_frame(av->video_codec_context, av->frame, &frame_size, packet)) {
		double pts = (((double)av->frame->pts)*(double)av->video_stream->time_base.num)/av->video_stream->time_base.den;
		//fprintf(stderr, "%f\n", pts);
		// Drop frames with negative PTS values
		if(pts < 0.0) {
			// do nothing
			return;
		}
		if((settings->video_frame_count) >= 1 && pts < av->video_next_pts) {
			// do nothing
			return;
		}
		if((settings->video_frame_count) < 1) {
			av->video_next_pts = pts;
		}

		double pts_step = ((double)1.0*(double)settings->video_fps_den)/(double)settings->video_fps_num;
		//fprintf(stderr, "%d %f %f %f\n", (settings->video_frame_count), pts, av->video_next_pts, pts_step);
		av->video_next_pts += pts_step;
		// FIXME: increasing framerate doesn't fill it in with duplicate frames!
		assert(av->video_next_pts > pts);
		//size_t buffer_size = frame_count_mul;
		//buffer[0] = malloc(buffer_size);
		//memset(buffer[0], 0, buffer_size);
		settings->video_frames = realloc(settings->video_frames, (settings->video_frame_count + 1) * av->video_frame_dst_size);
		int dst_strides[1] = {
			settings->video_width*4,
		};
		uint8_t *dst_pointers[1] = {
			(settings->video_frames) + av->video_frame_dst_size*(settings->video_frame_count),
		};
		sws_scale(av->scaler, av->frame->data, av->frame->linesize, 0, av->frame->height, dst_pointers, dst_strides);

		settings->video_frame_count += 1;
		//free(buffer[0]);
	}
}

static void poll_av_packet(settings_t *settings, AVPacket *packet)
{
	av_decoder_state_t* av = &(settings->decoder_state_av);

	if (packet->stream_index == av->audio_stream_index) {
		poll_av_packet_audio(settings, packet);
	}
	else if (packet->stream_index == av->video_stream_index) {
		poll_av_packet_video(settings, packet);
	}
}

bool poll_av_data(settings_t *settings)
{
	av_decoder_state_t* av = &(settings->decoder_state_av);
	AVPacket packet;

	if (av_read_frame(av->format, &packet) >= 0) {
		poll_av_packet(settings, &packet);
		av_packet_unref(&packet);
		return true;
	} else {
		// out is always padded out with 4032 "0" samples, this makes calculations elsewhere easier
		memset((settings->audio_samples) + (settings->audio_sample_count), 0, 4032 * av->sample_count_mul * sizeof(int16_t));

		return false;
	}
}

bool ensure_av_data(settings_t *settings, int needed_audio_samples, int needed_video_frames)
{
	//
	av_decoder_state_t* av = &(settings->decoder_state_av);


	while (settings->audio_sample_count < needed_audio_samples || settings->video_frame_count < needed_video_frames) {
		//fprintf(stderr, "ensure %d -> %d, %d -> %d\n", settings->audio_sample_count, needed_audio_samples, settings->video_frame_count, needed_video_frames);
		if(!poll_av_data(settings)) {
			//fprintf(stderr, "cannot ensure\n");
			return false;
		}
	}
	//fprintf(stderr, "ensure %d -> %d, %d -> %d\n", settings->audio_sample_count, needed_audio_samples, settings->video_frame_count, needed_video_frames);

	return true;
}

void pull_all_av_data(settings_t *settings)
{
	while (poll_av_data(settings)) {
		// do nothing
	}

	fprintf(stderr, "Loaded %d samples.\n", settings->audio_sample_count);
	fprintf(stderr, "Loaded %d frames.\n", settings->video_frame_count);
}

void retire_av_data(settings_t *settings, int retired_audio_samples, int retired_video_frames)
{
	av_decoder_state_t* av = &(settings->decoder_state_av);

	//fprintf(stderr, "retire %d -> %d, %d -> %d\n", settings->audio_sample_count, retired_audio_samples, settings->video_frame_count, retired_video_frames);
	assert(retired_audio_samples <= settings->audio_sample_count);
	assert(retired_video_frames <= settings->video_frame_count);

	int sample_size = sizeof(int16_t);
	if (settings->audio_sample_count > retired_audio_samples) {
		memmove(settings->audio_samples, settings->audio_samples + retired_audio_samples, (settings->audio_sample_count - retired_audio_samples)*sample_size);
		settings->audio_sample_count -= retired_audio_samples;
	}

	int frame_size = av->video_frame_dst_size;
	if (settings->video_frame_count > retired_video_frames) {
		memmove(settings->video_frames, settings->video_frames + retired_video_frames*frame_size, (settings->video_frame_count - retired_video_frames)*frame_size);
		settings->video_frame_count -= retired_video_frames;
	}
}

void close_av_data(settings_t *settings)
{
	av_decoder_state_t* av = &(settings->decoder_state_av);

	av_frame_free(&(av->frame));
	swr_free(&(av->resampler));
	avcodec_close(av->audio_codec_context);
	avcodec_free_context(&(av->audio_codec_context));
	avformat_free_context(av->format);

	if(settings->audio_samples != NULL) {
		free(settings->audio_samples);
		settings->audio_samples = NULL;
	}
	if(settings->video_frames != NULL) {
		free(settings->video_frames);
		settings->video_frames = NULL;
	}
}
