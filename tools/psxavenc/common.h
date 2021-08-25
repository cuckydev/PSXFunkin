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

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "libpsxav/libpsxav.h"

#define FORMAT_XA 0
#define FORMAT_XACD 1
#define FORMAT_SPU 2
#define FORMAT_STR2 3

#define MAX_UNMUXED_BLOCKS 9
typedef struct {
	int frame_index;
	int frame_block_index;
	int frame_block_count;
	int frame_block_base_overflow;
	int frame_block_overflow_num;
	int frame_block_overflow_den;
	uint16_t bits_value;
	int bits_left;
	uint8_t unmuxed[2016*MAX_UNMUXED_BLOCKS];
	int bytes_used;
	int blocks_used;
	int uncomp_hwords_used;
	int quant_scale;
	int32_t *dct_block_lists[6];
} vid_encoder_state_t;

typedef struct {
	int video_frame_src_size;
	int video_frame_dst_size;
	int audio_stream_index;
	int video_stream_index;
	AVFormatContext* format;
	AVStream* audio_stream;
	AVStream* video_stream;
	AVCodecContext* audio_codec_context;
	AVCodecContext* video_codec_context;
	AVCodec* audio_codec;
	AVCodec* video_codec;
	struct SwrContext* resampler;
	struct SwsContext* scaler;
	AVFrame* frame;

	int sample_count_mul;

	double video_next_pts;
} av_decoder_state_t;

typedef struct {
	int format; // FORMAT_*
	bool stereo; // false or true
	int frequency; // 18900 or 37800 Hz
	int bits_per_sample; // 4 or 8
	int file_number; // 00-FF
	int channel_number; // 00-1F

	int video_width;
	int video_height;
	int video_fps_num; // FPS numerator
	int video_fps_den; // FPS denominator

	int16_t *audio_samples;
	int audio_sample_count;
	uint8_t *video_frames;
	int video_frame_count;

	av_decoder_state_t decoder_state_av;

	vid_encoder_state_t state_vid;
} settings_t;

// cdrom.c
void init_sector_buffer_video(uint8_t *buffer, settings_t *settings);
void calculate_edc_data(uint8_t *buffer);

// decoding.c
bool open_av_data(const char *filename, settings_t *settings);
bool poll_av_data(settings_t *settings);
bool ensure_av_data(settings_t *settings, int needed_audio_samples, int needed_video_frames);
void pull_all_av_data(settings_t *settings);
void retire_av_data(settings_t *settings, int retired_audio_samples, int retired_video_frames);
void close_av_data(settings_t *settings);

// filefmt.c
void encode_file_spu(int16_t *audio_samples, int audio_sample_count, settings_t *settings, FILE *output);
void encode_file_xa(int16_t *audio_samples, int audio_sample_count, settings_t *settings, FILE *output);
void encode_file_str(settings_t *settings, FILE *output);

// mdec.c
void encode_block_str(uint8_t *video_frames, int video_frame_count, uint8_t *output, settings_t *settings);
