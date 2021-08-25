/*
xainterleave: simple sector interleaving tool

Copyright (c) 2019 Adrian "asie" Siekierka

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

This tool has been modified to only use 'just XA' mode, refer to .txt file automatically by input, and assume files referenced in .txt to be in the same directory as the .txt
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENTRY_MAX 64

#define TYPE_NULL 0
#define TYPE_RAW 1
#define TYPE_XA 2
#define TYPE_XACD 3

typedef struct {
	int sectors, type;

	FILE *file;

	int xa_file;
	int xa_channel;
} entry_t;

static entry_t entries[ENTRY_MAX];

int parse(char *filename) {
	entry_t e;
	char type_str[65];
	char fn_str[257];
	int entry_count = 0;
	FILE *file = fopen(filename, "r");
	if (file == NULL) return 0;
	
	char *cute = filename + strlen(filename) - 1;
	while (cute >= filename && *cute != '/' && *cute != '\\')
		cute--;
	cute[1] = '\0';

	while (fscanf(file, " %d %64s", &(e.sectors), type_str) > 0) {
		if (strcmp(type_str, "null") == 0) e.type = TYPE_NULL;
		else if (strcmp(type_str, "raw") == 0) e.type = TYPE_RAW;
		else if (strcmp(type_str, "xacd") == 0) e.type = TYPE_XACD;
		else if (strcmp(type_str, "xa") == 0) e.type = TYPE_XA;
		else { fprintf(stderr, "Unknown type: %s\n", type_str); continue; }

		switch (e.type) {
			case TYPE_RAW:
			case TYPE_XA:
			case TYPE_XACD:
				if (fscanf(file, " %256s", fn_str) > 0) {
					char *npath = malloc(strlen(filename) + strlen(fn_str) + 1);
					if (npath == NULL)
						return 0;
					sprintf(npath, "%s%s", filename, fn_str);
					e.file = fopen(npath, "rb");
					free(npath);
					if (e.file == NULL) return 0;
				} else return 0;
				break;
		}

		switch (e.type) {
			case TYPE_XA:
			case TYPE_XACD:
				if (fscanf(file, " %d %d", &(e.xa_file), &(e.xa_channel)) > 0) {
					// nop
				} else {
					e.xa_file = 0;
					e.xa_channel = 0;
				}
				break;
		}

		entries[entry_count] = e;
		entry_count++;
	}

	fclose(file);
	return entry_count;
}

int main(int argc, char** argv) {
	uint8_t buffer[2352];

	if (argc < 2) {
		fprintf(stderr, "Usage: xainterleave <out.xa>\n");
		return 1;
	}

	int mode = 1; //Just XA

	char *txtpath = malloc(strlen(argv[1]) + 5);
	sprintf(txtpath, "%s.txt", argv[1]);

	int entry_count = parse(txtpath);
	
	free(txtpath);
	if (entry_count <= 0) {
		fprintf(stderr, "Empty manifest?\n");
		return 1;
	}

	int sector_div = 0;
	for (int i = 0; i < entry_count; i++) {
		sector_div += entries[i].sectors;
	}
	printf("Interleaving into %d-sector chunks\n", sector_div);

	FILE *output = fopen(argv[1], "wb");

	while (1) {
		int can_read = 0;
		for (int i = 0; i < entry_count; i++) {
			entry_t *e = &entries[i];
			if (e->file != NULL) {
				if (!feof(e->file)) can_read++;
			}
		}
		if (can_read <= 0) break;

		for (int i = 0; i < entry_count; i++) {
			entry_t *e = &entries[i];
			for (int is = 0; is < e->sectors; is++) {
				int write_null = 0;
				switch (e->type) {
					case TYPE_NULL:
						write_null = 1;
						break;
					case TYPE_RAW:
						if (mode != 0) {
							fprintf(stderr, "Can only write raw sectors in raw sector mode\n");
							return 1;
						}
						if (!fread(buffer, 2352, 1, e->file)) { write_null = 1; break; }
						fwrite(buffer, 2352, 1, output);
						break;
					case TYPE_XA:
					case TYPE_XACD:
						if (e->type == TYPE_XACD) {
							if (mode != 0) {
								fprintf(stderr, "Can only write raw sectors in raw sector mode\n");
								return 1;
							}
							if (!fread(buffer, 2352, 1, e->file)) { write_null = 1; break; }
						} else {
							if (!fread(buffer + 0x10, 2336, 1, e->file)) { write_null = 1; break; }
							memset(buffer, 0, 15);
							buffer[15] = 0x02;
						}
						if (e->xa_file >= 0) buffer[0x010] = buffer[0x014] = e->xa_file;
						if (e->xa_channel >= 0) buffer[0x011] = buffer[0x015] = e->xa_channel & 0x1F;
						buffer[0x92F] = 0xFF; // make pscd-new generate EDC
						if (mode == 0) {
							fwrite(buffer, 2352, 1, output);
						}
						else if (mode == 1) {
							fwrite(buffer + 0x10, 2336, 1, output);
						}
						break;
				}

				if (write_null) {
					for (int j = 0; j < (mode == 0 ? 2352 : 2336); j++) fputc(0, output);
				}
			}
		}
	}

	fclose(output);
	return 0;
}
