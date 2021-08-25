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

#ifndef __LIBPSXAV_H__
#define __LIBPSXAV_H__

#include <stdbool.h>
#include <stdint.h>

// audio.c

#define PSX_AUDIO_XA_FREQ_SINGLE 18900
#define PSX_AUDIO_XA_FREQ_DOUBLE 37800

typedef enum {
	PSX_AUDIO_XA_FORMAT_XA, // .xa file
	PSX_AUDIO_XA_FORMAT_XACD // 2352-byte sector
} psx_audio_xa_format_t;

typedef struct {
	psx_audio_xa_format_t format;
	bool stereo; // false or true
	int frequency; // 18900 or 37800 Hz
	int bits_per_sample; // 4 or 8
	int file_number; // 00-FF
	int channel_number; // 00-1F
} psx_audio_xa_settings_t;

typedef struct {
	int qerr; // quanitisation error
	uint64_t mse; // mean square error
	int prev1, prev2;
} psx_audio_encoder_channel_state_t;

typedef struct {
	psx_audio_encoder_channel_state_t left;
	psx_audio_encoder_channel_state_t right;
} psx_audio_encoder_state_t;

#define PSX_AUDIO_SPU_LOOP_END 1
#define PSX_AUDIO_SPU_LOOP_REPEAT 3
#define PSX_AUDIO_SPU_LOOP_START 4

uint32_t psx_audio_xa_get_buffer_size(psx_audio_xa_settings_t settings, int sample_count);
uint32_t psx_audio_spu_get_buffer_size(int sample_count);
uint32_t psx_audio_xa_get_buffer_size_per_sector(psx_audio_xa_settings_t settings);
uint32_t psx_audio_spu_get_buffer_size_per_block(void);
uint32_t psx_audio_xa_get_samples_per_sector(psx_audio_xa_settings_t settings);
uint32_t psx_audio_spu_get_samples_per_block(void);
int psx_audio_xa_encode(psx_audio_xa_settings_t settings, psx_audio_encoder_state_t *state, int16_t* samples, int sample_count, uint8_t *output);
int psx_audio_xa_encode_simple(psx_audio_xa_settings_t settings, int16_t* samples, int sample_count, uint8_t *output);
int psx_audio_spu_encode(psx_audio_encoder_state_t *state, int16_t* samples, int sample_count, uint8_t *output);
int psx_audio_spu_encode_simple(int16_t* samples, int sample_count, uint8_t *output, int loop_start);
int psx_audio_xa_encode_finalize(psx_audio_xa_settings_t settings, uint8_t *output, int output_length);
void psx_audio_spu_set_flag_at_sample(uint8_t* spu_data, int sample_pos, int flag);

// cdrom.c

#define PSX_CDROM_SECTOR_SIZE 2352

typedef enum {
	PSX_CDROM_SECTOR_TYPE_MODE1,
	PSX_CDROM_SECTOR_TYPE_MODE2_FORM1,
	PSX_CDROM_SECTOR_TYPE_MODE2_FORM2
} psx_cdrom_sector_type_t;

void psx_cdrom_calculate_checksums(uint8_t *sector, psx_cdrom_sector_type_t type);

#endif /* __LIBPSXAV_H__ */