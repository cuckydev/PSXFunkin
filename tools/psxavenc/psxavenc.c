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

void print_help(void) {
	fprintf(stderr, "Usage: psxavenc [-f freq] [-b bitdepth] [-c channels] [-F num] [-C num] [-t xa|xacd|spu|str2] <in> <out>\n\n");
	fprintf(stderr, "    -f freq          Use specified frequency\n");
	fprintf(stderr, "    -t format        Use specified output type:\n");
	fprintf(stderr, "                       xa     [A.] .xa 2336-byte sectors\n");
	fprintf(stderr, "                       xacd   [A.] .xa 2352-byte sectors\n");
	fprintf(stderr, "                       spu    [A.] raw SPU-ADPCM data\n");
	fprintf(stderr, "                       str2   [AV] v2 .str video 2352-byte sectors\n");
	fprintf(stderr, "    -b bitdepth      Use specified bit depth (only 4 bits supported)\n");
	fprintf(stderr, "    -c channels      Use specified channel count (1 or 2)\n");
	fprintf(stderr, "    -F num           [.xa] Set the file number to num (0-255)\n");
	fprintf(stderr, "    -C num           [.xa] Set the channel number to num (0-31)\n");
}

int parse_args(settings_t* settings, int argc, char** argv) {
	int c;
	while ((c = getopt(argc, argv, "t:f:b:c:F:C:")) != -1) {
		switch (c) {
			case 't': {
				if (strcmp(optarg, "xa") == 0) {
					settings->format = FORMAT_XA;
				} else if (strcmp(optarg, "xacd") == 0) {
					settings->format = FORMAT_XACD;
				} else if (strcmp(optarg, "spu") == 0) {
					settings->format = FORMAT_SPU;
				} else if (strcmp(optarg, "str2") == 0) {
					settings->format = FORMAT_STR2;
				} else {
					fprintf(stderr, "Invalid format: %s\n", optarg);
					return -1;
				}
			} break;
			case 'f': {
				settings->frequency = atoi(optarg);
			} break;
			case 'b': {
				settings->bits_per_sample = atoi(optarg);
				if (settings->bits_per_sample != 4) {
					fprintf(stderr, "Invalid bit depth: %d\n", settings->frequency);
					return -1;
				}
			} break;
			case 'c': {
				int ch = atoi(optarg);
				if (ch <= 0 || ch > 2) {
					fprintf(stderr, "Invalid channel count: %d\n", ch);
					return -1;
				}
				settings->stereo = (ch == 2 ? 1 : 0);
			} break;
			case 'F': {
				settings->file_number = atoi(optarg);
				if (settings->file_number < 0 || settings->file_number > 255) {
					fprintf(stderr, "Invalid file number: %d\n", settings->file_number);
					return -1;
				}
			} break;
			case 'C': {
				settings->channel_number = atoi(optarg);
				if (settings->channel_number < 0 || settings->channel_number > 31) {
					fprintf(stderr, "Invalid channel number: %d\n", settings->channel_number);
					return -1;
				}
			} break;
			case '?':
			case 'h': {
				print_help();
				return -1;
			} break;
		}
	}

	if (settings->format == FORMAT_XA || settings->format == FORMAT_XACD) {
		if (settings->frequency != PSX_AUDIO_XA_FREQ_SINGLE && settings->frequency != PSX_AUDIO_XA_FREQ_DOUBLE) {
			fprintf(stderr, "Invalid frequency: %d Hz\n", settings->frequency);
			return -1;
		}
	}

	if (settings->format == FORMAT_SPU) {
		settings->stereo = false;
	}

	return optind;
}

int main(int argc, char **argv) {
	settings_t settings;
	int arg_offset;
	FILE* output;

	memset(&settings,0,sizeof(settings_t));

	settings.file_number = 0;
	settings.channel_number = 0;
	settings.stereo = true;
	settings.frequency = PSX_AUDIO_XA_FREQ_DOUBLE;
	settings.bits_per_sample = 4;

	settings.video_width = 320;
	settings.video_height = 240;

	settings.audio_samples = NULL;
	settings.audio_sample_count = 0;
	settings.video_frames = NULL;
	settings.video_frame_count = 0;

	// TODO: make this adjustable
	// also for some reason ffmpeg seems to hard-code the framerate to 15fps
	settings.video_fps_num = 15;
	settings.video_fps_den = 1;
	for(int i = 0; i < 6; i++) {
		settings.state_vid.dct_block_lists[i] = NULL;
	}

	arg_offset = parse_args(&settings, argc, argv);
	if (arg_offset < 0) {
		return 1;
	} else if (argc < arg_offset + 2) {
		print_help();
		return 1;
	}

	fprintf(stderr, "Using settings: %d Hz @ %d bit depth, %s. F%d C%d\n",
		settings.frequency, settings.bits_per_sample,
		settings.stereo ? "stereo" : "mono",
		settings.file_number, settings.channel_number
	);

	bool did_open_data = open_av_data(argv[arg_offset + 0], &settings);
	if (!did_open_data) {
		fprintf(stderr, "Could not open input file!\n");
		return 1;
	}

	output = fopen(argv[arg_offset + 1], "wb");
	if (output == NULL) {
		fprintf(stderr, "Could not open output file!\n");
		return 1;
	}

	int av_sample_mul = settings.stereo ? 2 : 1;

	switch (settings.format) {
		case FORMAT_XA:
		case FORMAT_XACD:
			pull_all_av_data(&settings);
			encode_file_xa(settings.audio_samples, settings.audio_sample_count / av_sample_mul, &settings, output);
			break;
		case FORMAT_SPU:
			pull_all_av_data(&settings);
			encode_file_spu(settings.audio_samples, settings.audio_sample_count / av_sample_mul, &settings, output);
			break;
		case FORMAT_STR2:
			encode_file_str(&settings, output);
			break;
	}

	fclose(output);
	close_av_data(&settings);
	return 0;
}
