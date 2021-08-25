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

static psx_audio_xa_settings_t settings_to_libpsxav_xa_audio(settings_t *settings) {
	psx_audio_xa_settings_t new_settings;
	new_settings.bits_per_sample = settings->bits_per_sample;
	new_settings.frequency = settings->frequency;
	new_settings.stereo = settings->stereo;
	new_settings.file_number = settings->file_number;
	new_settings.channel_number = settings->channel_number;

	switch (settings->format) {
		case FORMAT_XA:
			new_settings.format = PSX_AUDIO_XA_FORMAT_XA;
			break;
		default:
			new_settings.format = PSX_AUDIO_XA_FORMAT_XACD;
			break;
	}

	return new_settings;
};

void encode_file_spu(int16_t *audio_samples, int audio_sample_count, settings_t *settings, FILE *output) {
	psx_audio_encoder_state_t audio_state;	
	int audio_samples_per_block = psx_audio_spu_get_samples_per_block();
	uint8_t buffer[16];

	memset(&audio_state, 0, sizeof(psx_audio_encoder_state_t));

	for (int i = 0; i < audio_sample_count; i += audio_samples_per_block) {
		int samples_length = audio_sample_count - i;
		if (samples_length > audio_samples_per_block) samples_length = audio_samples_per_block;
		int length = psx_audio_spu_encode(&audio_state, audio_samples + i, samples_length, buffer);
		if (i == 0) {
			buffer[1] = PSX_AUDIO_SPU_LOOP_START;
		} else if ((i + audio_samples_per_block) >= audio_sample_count) {
			buffer[1] = PSX_AUDIO_SPU_LOOP_END;
		}
		fwrite(buffer, length, 1, output);
	}
}

void encode_file_xa(int16_t *audio_samples, int audio_sample_count, settings_t *settings, FILE *output) {
	psx_audio_xa_settings_t xa_settings = settings_to_libpsxav_xa_audio(settings);
	psx_audio_encoder_state_t audio_state;	
	int audio_samples_per_sector = psx_audio_xa_get_samples_per_sector(xa_settings);
	int av_sample_mul = settings->stereo ? 2 : 1;
	uint8_t buffer[2352];

	memset(&audio_state, 0, sizeof(psx_audio_encoder_state_t));

	for (int i = 0; i < audio_sample_count; i += audio_samples_per_sector) {
		int samples_length = audio_sample_count - i;
		if (samples_length > audio_samples_per_sector) samples_length = audio_samples_per_sector;
		int length = psx_audio_xa_encode(xa_settings, &audio_state, audio_samples + (i * av_sample_mul), samples_length, buffer);
		if ((i + audio_samples_per_sector) >= audio_sample_count) {
			psx_audio_xa_encode_finalize(xa_settings, buffer, length);
		}
		fwrite(buffer, length, 1, output);
	}
}

void encode_file_str(settings_t *settings, FILE *output) {
	uint8_t buffer[2352*8];
	psx_audio_xa_settings_t xa_settings = settings_to_libpsxav_xa_audio(settings);
	psx_audio_encoder_state_t audio_state;	
	int audio_samples_per_sector = psx_audio_xa_get_samples_per_sector(xa_settings);
	int av_sample_mul = settings->stereo ? 2 : 1;

	memset(&audio_state, 0, sizeof(psx_audio_encoder_state_t));

	settings->state_vid.frame_index = 0;
	settings->state_vid.bits_value = 0;
	settings->state_vid.bits_left = 16;
	settings->state_vid.frame_block_index = 0;
	settings->state_vid.frame_block_count = 0;

	settings->state_vid.frame_block_overflow_num = 0;

	// Number of total sectors per second: 150
	// Proportion of sectors for video due to A/V interleave: 7/8
	// 15FPS = (150*7/8/15) = 8.75 blocks per frame
	settings->state_vid.frame_block_base_overflow = 150*7*settings->video_fps_den;
	settings->state_vid.frame_block_overflow_den = 8*settings->video_fps_num;
	//fprintf(stderr, "%f\n", ((double)settings->state_vid.frame_block_base_overflow)/((double)settings->state_vid.frame_block_overflow_den)); abort();

	// FIXME: this needs an extra frame to prevent A/V desync
	const int frames_needed = 2;
	for (int j = 0; ensure_av_data(settings, audio_samples_per_sector*av_sample_mul*frames_needed, 1*frames_needed); j+=18) {
		psx_audio_xa_encode(xa_settings, &audio_state, settings->audio_samples, audio_samples_per_sector, buffer + 2352 * 7);
		
		// TODO: the final buffer
		for(int k = 0; k < 7; k++) {
			init_sector_buffer_video(buffer + 2352*k, settings);
		}
		encode_block_str(settings->video_frames, settings->video_frame_count, buffer, settings);
		for(int k = 0; k < 8; k++) {
			int t = k + (j/18)*8 + 75*2;

			// Put the time in
			buffer[0x00C + 2352*k] = ((t/75/60)%10)|(((t/75/60)/10)<<4);
			buffer[0x00D + 2352*k] = (((t/75)%60)%10)|((((t/75)%60)/10)<<4);
			buffer[0x00E + 2352*k] = ((t%75)%10)|(((t%75)/10)<<4);

			if(k != 7) {
				calculate_edc_data(buffer + 2352*k);
			}
		}
		retire_av_data(settings, audio_samples_per_sector*av_sample_mul, 0);
		fwrite(buffer, 2352*8, 1, output);
	}
}
