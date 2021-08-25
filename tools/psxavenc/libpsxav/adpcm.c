/*
libpsxav: MDEC video + SPU/XA-ADPCM audio library

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
*/

#include <assert.h>
#include <string.h>
#include "libpsxav.h"

#define ADPCM_FILTER_COUNT 5
#define XA_ADPCM_FILTER_COUNT 4
#define SPU_ADPCM_FILTER_COUNT 5

static const int16_t filter_k1[ADPCM_FILTER_COUNT] = {0, 60, 115, 98, 122};
static const int16_t filter_k2[ADPCM_FILTER_COUNT] = {0, 0, -52, -55, -60};

static int find_min_shift(const psx_audio_encoder_channel_state_t *state, int16_t *samples, int pitch, int filter) {
	// Assumption made:
	//
	// There is value in shifting right one step further to allow the nibbles to clip.
	// However, given a possible shift value, there is no value in shifting one step less.
	//
	// Having said that, this is not a completely accurate model of the encoder,
	// so maybe we will need to shift one step less.
	//
	int prev1 = state->prev1;
	int prev2 = state->prev2;
	int k1 = filter_k1[filter];
	int k2 = filter_k2[filter];

	int right_shift = 0;

	int32_t s_min = 0;
	int32_t s_max = 0;
	for (int i = 0; i < 28; i++) {
		int32_t raw_sample = samples[i * pitch];
		int32_t previous_values = (k1*prev1 + k2*prev2 + (1<<5))>>6;
		int32_t sample = raw_sample - previous_values;
		if (sample < s_min) { s_min = sample; }
		if (sample > s_max) { s_max = sample; }
		prev2 = prev1;
		prev1 = raw_sample;
	}
	while(right_shift < 12 && (s_max>>right_shift) > +0x7) { right_shift += 1; };
	while(right_shift < 12 && (s_min>>right_shift) < -0x8) { right_shift += 1; };

	int min_shift = 12 - right_shift;
	assert(0 <= min_shift && min_shift <= 12);
	return min_shift;
}

static uint8_t attempt_to_encode_nibbles(psx_audio_encoder_channel_state_t *outstate, const psx_audio_encoder_channel_state_t *instate, int16_t *samples, int sample_limit, int pitch, uint8_t *data, int data_shift, int data_pitch, int filter, int sample_shift) {
	uint8_t nondata_mask = ~(0x0F << data_shift);
	int min_shift = sample_shift;
	int k1 = filter_k1[filter];
	int k2 = filter_k2[filter];

	uint8_t hdr = (min_shift & 0x0F) | (filter << 4);

	if (outstate != instate) {
		memcpy(outstate, instate, sizeof(psx_audio_encoder_channel_state_t));
	}

	outstate->mse = 0;

	for (int i = 0; i < 28; i++) {
		int32_t sample = ((i * pitch) >= sample_limit ? 0 : samples[i * pitch]) + outstate->qerr;
		int32_t previous_values = (k1*outstate->prev1 + k2*outstate->prev2 + (1<<5))>>6;
		int32_t sample_enc = sample - previous_values;
		sample_enc <<= min_shift;
		sample_enc += (1<<(12-1));
		sample_enc >>= 12;
		if(sample_enc < -8) { sample_enc = -8; }
		if(sample_enc > +7) { sample_enc = +7; }
		sample_enc &= 0xF;

		int32_t sample_dec = (int16_t) ((sample_enc&0xF) << 12);
		sample_dec >>= min_shift;
		sample_dec += previous_values;
		if (sample_dec > +0x7FFF) { sample_dec = +0x7FFF; }
		if (sample_dec < -0x8000) { sample_dec = -0x8000; }
		int64_t sample_error = sample_dec - sample;

		assert(sample_error < (1<<30));
		assert(sample_error > -(1<<30));

		data[i * data_pitch] = (data[i * data_pitch] & nondata_mask) | (sample_enc << data_shift);
		// FIXME: dithering is hard to predict
		//outstate->qerr += sample_error;
		outstate->mse += ((uint64_t)sample_error) * (uint64_t)sample_error;

		outstate->prev2 = outstate->prev1;
		outstate->prev1 = sample_dec;
	}

	return hdr;
}

static uint8_t encode_nibbles(psx_audio_encoder_channel_state_t *state, int16_t *samples, int sample_limit, int pitch, uint8_t *data, int data_shift, int data_pitch, int filter_count) {
    psx_audio_encoder_channel_state_t proposed;
	int64_t best_mse = ((int64_t)1<<(int64_t)50);
	int best_filter = 0;
	int best_sample_shift = 0;

	for (int filter = 0; filter < filter_count; filter++) {
		int true_min_shift = find_min_shift(state, samples, pitch, filter);

		// Testing has shown that the optimal shift can be off the true minimum shift
		// by 1 in *either* direction.
		// This is NOT the case when dither is used.
		int min_shift = true_min_shift - 1;
		int max_shift = true_min_shift + 1;
		if (min_shift < 0) { min_shift = 0; }
		if (max_shift > 12) { max_shift = 12; }

		for (int sample_shift = min_shift; sample_shift <= max_shift; sample_shift++) {
			// ignore header here
			attempt_to_encode_nibbles(
				&proposed, state,
				samples, sample_limit, pitch,
				data, data_shift, data_pitch,
				filter, sample_shift);

			if (best_mse > proposed.mse) {
				best_mse = proposed.mse;
				best_filter = filter;
				best_sample_shift = sample_shift;
			}
		}
	}

	// now go with the encoder
	return attempt_to_encode_nibbles(
		state, state,
		samples, sample_limit, pitch,
		data, data_shift, data_pitch,
		best_filter, best_sample_shift);
}

static void encode_block_xa(int16_t *audio_samples, int audio_samples_limit, uint8_t *data, psx_audio_xa_settings_t settings, psx_audio_encoder_state_t *state) {
	if (settings.bits_per_sample == 4) {
		if (settings.stereo) {
			data[0]  = encode_nibbles(&(state->left), audio_samples, audio_samples_limit,           2, data + 0x10, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[1]  = encode_nibbles(&(state->right), audio_samples + 1, audio_samples_limit - 1,        2, data + 0x10, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[2]  = encode_nibbles(&(state->left), audio_samples + 56, audio_samples_limit - 56,       2, data + 0x11, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[3]  = encode_nibbles(&(state->right), audio_samples + 56 + 1, audio_samples_limit - 56 - 1,  2, data + 0x11, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[8]  = encode_nibbles(&(state->left), audio_samples + 56*2, audio_samples_limit - 56*2,    2, data + 0x12, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[9]  = encode_nibbles(&(state->right), audio_samples + 56*2 + 1, audio_samples_limit - 56*2 - 1, 2, data + 0x12, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[10] = encode_nibbles(&(state->left), audio_samples + 56*3, audio_samples_limit - 56*3,     2, data + 0x13, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[11] = encode_nibbles(&(state->right), audio_samples + 56*3 + 1, audio_samples_limit - 56*3 - 1, 2, data + 0x13, 4, 4, XA_ADPCM_FILTER_COUNT);
		} else {
			data[0]  = encode_nibbles(&(state->left), audio_samples, audio_samples_limit,           1, data + 0x10, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[1]  = encode_nibbles(&(state->right), audio_samples + 28, audio_samples_limit - 28,       1, data + 0x10, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[2]  = encode_nibbles(&(state->left), audio_samples + 28*2, audio_samples_limit - 28*2,     1, data + 0x11, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[3]  = encode_nibbles(&(state->right), audio_samples + 28*3, audio_samples_limit - 28*3,     1, data + 0x11, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[8]  = encode_nibbles(&(state->left), audio_samples + 28*4, audio_samples_limit - 28*4,     1, data + 0x12, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[9]  = encode_nibbles(&(state->right), audio_samples + 28*5, audio_samples_limit - 28*5,     1, data + 0x12, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[10] = encode_nibbles(&(state->left), audio_samples + 28*6, audio_samples_limit - 28*6,     1, data + 0x13, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[11] = encode_nibbles(&(state->right), audio_samples + 28*7, audio_samples_limit - 28*7,     1, data + 0x13, 4, 4, XA_ADPCM_FILTER_COUNT);
		}
	} else {
/*		if (settings->stereo) {
			data[0]  = encode_bytes(audio_samples,            2, data + 0x10);
			data[1]  = encode_bytes(audio_samples + 1,        2, data + 0x11);
			data[2]  = encode_bytes(audio_samples + 56,       2, data + 0x12);
			data[3]  = encode_bytes(audio_samples + 57,       2, data + 0x13);
		} else {
			data[0]  = encode_bytes(audio_samples,            1, data + 0x10);
			data[1]  = encode_bytes(audio_samples + 28,       1, data + 0x11);
			data[2]  = encode_bytes(audio_samples + 56,       1, data + 0x12);
			data[3]  = encode_bytes(audio_samples + 84,       1, data + 0x13);
		} */
	}
}

uint32_t psx_audio_xa_get_buffer_size(psx_audio_xa_settings_t settings, int sample_count) {
	int sample_pitch = psx_audio_xa_get_samples_per_sector(settings);
	int xa_sectors = ((sample_count + sample_pitch - 1) / sample_pitch);
	int xa_sector_size = psx_audio_xa_get_buffer_size_per_sector(settings);
	return xa_sectors * xa_sector_size;
}

uint32_t psx_audio_spu_get_buffer_size(int sample_count) {
	return ((sample_count + 27) / 28) << 4;
}

uint32_t psx_audio_xa_get_buffer_size_per_sector(psx_audio_xa_settings_t settings) {
	return settings.format == PSX_AUDIO_XA_FORMAT_XA ? 2336 : 2352;
}

uint32_t psx_audio_spu_get_buffer_size_per_block(void) {
	return 16;
}

uint32_t psx_audio_xa_get_samples_per_sector(psx_audio_xa_settings_t settings) {
	return (((settings.bits_per_sample == 8) ? 112 : 224) >> (settings.stereo ? 1 : 0)) * 18;
}

uint32_t psx_audio_spu_get_samples_per_block(void) {
	return 28;
}

static void psx_audio_xa_encode_init_sector(uint8_t *buffer, psx_audio_xa_settings_t settings) {
	if (settings.format == PSX_AUDIO_XA_FORMAT_XACD) {
		memset(buffer, 0, 2352);
		memset(buffer+0x001, 0xFF, 10);
		buffer[0x00F] = 0x02;
	} else {
		memset(buffer + 0x10, 0, 2336);
	}

	buffer[0x010] = settings.file_number;
	buffer[0x011] = settings.channel_number & 0x1F;
	buffer[0x012] = 0x24 | 0x40;
	buffer[0x013] =
		(settings.stereo ? 1 : 0)
		| (settings.frequency >= PSX_AUDIO_XA_FREQ_DOUBLE ? 0 : 4)
		| (settings.bits_per_sample >= 8 ? 16 : 0);
	memcpy(buffer + 0x014, buffer + 0x010, 4);
}

int psx_audio_xa_encode(psx_audio_xa_settings_t settings, psx_audio_encoder_state_t *state, int16_t* samples, int sample_count, uint8_t *output) {
	int sample_jump = (settings.bits_per_sample == 8) ? 112 : 224;
	int i, j;
	int xa_sector_size = settings.format == PSX_AUDIO_XA_FORMAT_XA ? 2336 : 2352;
	int xa_offset = 2352 - xa_sector_size;
	uint8_t init_sector = 1;

	if (settings.stereo) { sample_count <<= 1; }
	
	for (i = 0, j = 0; i < sample_count || ((j % 18) != 0); i += sample_jump, j++) {
		uint8_t *sector_data = output + ((j/18) * xa_sector_size) - xa_offset;
		uint8_t *block_data = sector_data + 0x18 + ((j%18) * 0x80);

		if (init_sector) {
			psx_audio_xa_encode_init_sector(sector_data, settings);
			init_sector = 0;
		}

		encode_block_xa(samples + i, sample_count - i, block_data, settings, state);

		memcpy(block_data + 4, block_data, 4);
		memcpy(block_data + 12, block_data + 8, 4);

		if ((j+1)%18 == 0) {
			psx_cdrom_calculate_checksums(sector_data, PSX_CDROM_SECTOR_TYPE_MODE2_FORM2);
			init_sector = 1;
		}
	}

	return (((j + 17) / 18) * xa_sector_size);
}

int psx_audio_xa_encode_finalize(psx_audio_xa_settings_t settings, uint8_t *output, int output_length) {
	if (output_length >= 2336) {
		output[output_length - 2352 + 0x12] |= 0x80;
		output[output_length - 2352 + 0x18] |= 0x80;
	}
}

int psx_audio_xa_encode_simple(psx_audio_xa_settings_t settings, int16_t* samples, int sample_count, uint8_t *output) {
	psx_audio_encoder_state_t state;
	memset(&state, 0, sizeof(psx_audio_encoder_state_t));
	int length = psx_audio_xa_encode(settings, &state, samples, sample_count, output);
	psx_audio_xa_encode_finalize(settings, output, length);
	return length;
}

int psx_audio_spu_encode(psx_audio_encoder_state_t *state, int16_t* samples, int sample_count, uint8_t *output) {
	uint8_t prebuf[28];
	uint8_t *buffer = output;
	uint8_t *data;

	for (int i = 0; i < sample_count; i += 28, buffer += 16) {
		buffer[0] = encode_nibbles(&(state->left), samples + i, sample_count - i, 1, prebuf, 0, 1, SPU_ADPCM_FILTER_COUNT);
		buffer[1] = 0;

		for (int j = 0; j < 28; j+=2) {
			buffer[2 + (j>>1)] = (prebuf[j] & 0x0F) | (prebuf[j+1] << 4);
		}
	}

	return buffer - output;
}

int psx_audio_spu_encode_simple(int16_t* samples, int sample_count, uint8_t *output, int loop_start) {
	psx_audio_encoder_state_t state;
	memset(&state, 0, sizeof(psx_audio_encoder_state_t));
	int length = psx_audio_spu_encode(&state, samples, sample_count, output);

	if (length >= 32) {
		if (loop_start < 0) {
			output[1] = 4;
			output[length - 16 + 1] = 1;
		} else {
			psx_audio_spu_set_flag_at_sample(output, loop_start, 4);
			output[length - 16 + 1] = 3;
		}
	} else if (length >= 16) {
		output[1] = loop_start >= 0 ? 7 : 5;
	}

	return length;
}

void psx_audio_spu_set_flag_at_sample(uint8_t* spu_data, int sample_pos, int flag) {
	int buffer_pos = (sample_pos / 28) << 4;
	spu_data[buffer_pos + 1] = flag;
}
