/*
 * funkintimconv by Regan "CuckyDev" Green
 * Converts image files to TIM files for the Friday Night Funkin' PSX port
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.c"

typedef union
{
	struct
	{
		unsigned short r : 5;
		unsigned short g : 5;
		unsigned short b : 5;
		unsigned short i : 1;
	} c;
	uint16_t v;
} RGBI;

int main(int argc, char *argv[])
{
	//Read parameters
	if (argc < 3)
	{
		printf("usage: funkintimconv out.tim in.png\n");
		return 0;
	}
	
	const char *outpath = argv[1];
	const char *inpath = argv[2];
	
	char *txtpath = malloc(strlen(inpath) + 5);
	if (txtpath == NULL)
	{
		printf("Failed to allocate txt path\n");
		return 1;
	}
	sprintf(txtpath, "%s.txt", inpath);
	
	FILE *txtfp = fopen(txtpath, "r");
	free(txtpath);
	
	if (txtfp == NULL)
	{
		printf("Failed to open %s.txt\n", inpath);
		return 1;
	}
	
	int tex_x, tex_y, pal_x, pal_y, bpp;
	int txtread = fscanf(txtfp, "%d %d %d %d %d", &tex_x, &tex_y, &pal_x, &pal_y, &bpp);
	fclose(txtfp);
	
	if (txtread != 5)
	{
		printf("Failed to read parameters from %s.txt\n", inpath);
		return 1;
	}
	
	//Validate parameters
	int max_colour, width_shift;
	switch (bpp)
	{
		case 4:
			max_colour = 16;
			width_shift = 2;
			break;
		case 8:
			max_colour = 256;
			width_shift = 1;
			break;
		default:
			printf("Invalid bpp %d\n", bpp);
			return 1;
	}
	
	//Read image contents
	int tex_width, tex_height;
	stbi_uc *tex_data = stbi_load(inpath, &tex_width, &tex_height, NULL, 4);
	if (tex_data == NULL)
	{
		printf("Failed to read texture data from %s\n", inpath);
		return 1;
	}
	
	if (tex_width & ((1 << width_shift) - 1))
	{
		printf("Width %d can't properly be represented with bpp of %d\n", tex_width, bpp);
		stbi_image_free(tex_data);
		return 1;
	}
	
	//Convert image
	RGBI pal[256];
	int pals_i = 0;
	memset(pal, 0, sizeof(pal));
	
	size_t tex_size = ((tex_width << 1) >> width_shift) * tex_height;
	uint8_t *tex = malloc(tex_size);
	if (tex == NULL)
	{
		printf("Failed to allocate texture buffer\n");
		stbi_image_free(tex_data);
		return 1;
	}
	
	stbi_uc *tex_datap = tex_data;
	uint8_t *texp = tex;
	for (int i = tex_width * tex_height; i > 0; i--, tex_datap += 4)
	{
		//Get palette representation
		RGBI rep;
		if (tex_datap[3] & 0x80)
		{
			//Opaque
			rep.c.r = tex_datap[0] / 8;
			rep.c.g = tex_datap[1] / 8;
			rep.c.b = tex_datap[2] / 8;
			if (rep.c.b == 0)
				rep.c.b = 1; //Avoid bad transparency
			rep.c.i = 0;
		}
		else
		{
			//Transparent
			rep.c.r = 0;
			rep.c.g = 0;
			rep.c.b = 0;
			rep.c.i = 0;
		}
		
		//Get palette index
		int pal_i = 0;
		while (1)
		{
			if (pal_i >= max_colour)
			{
				printf("Image has more than %d colours\n", max_colour);
				free(tex);
				stbi_image_free(tex_data);
				return 1;
			}
			if (pal_i >= pals_i)
			{
				pal[pal_i].v = rep.v;
				pals_i++;
				break;
			}
			if (pal[pal_i].v == rep.v)
				break;
			pal_i++;
		}
		
		//Write pixel
		switch (bpp)
		{
			case 4:
				if (i & 1)
					*texp++ = *texp | (pal_i << 4);
				else
					*texp = pal_i;
				break;
			case 8:
				*texp++ = pal_i;
				break;
		}
	}
	stbi_image_free(tex_data);
	
	//Write output
	FILE *outfp = fopen(outpath, "wb");
	if (outfp == NULL)
	{
		printf("Failed to open %s\n", outpath);
		free(tex);
		return 1;
	}
	
	//Header
	fputc(0x10, outfp);
	fputc(0, outfp);
	fputc(0, outfp);
	fputc(0, outfp);
	switch (bpp)
	{
		case 4:
			fputc(0x08, outfp);
			break;
		case 8:
			fputc(0x09, outfp);
			break;
	}
	fputc(0, outfp);
	fputc(0, outfp);
	fputc(0, outfp);
	
	//CLUT
	pals_i = max_colour;
	uint32_t clut_length = 12 + 2 * pals_i;
	fputc(clut_length, outfp);
	fputc(clut_length >> 8, outfp);
	fputc(clut_length >> 16, outfp);
	fputc(clut_length >> 24, outfp);
	fputc(pal_x, outfp);
	fputc(pal_x >> 8, outfp);
	fputc(pal_y, outfp);
	fputc(pal_y >> 8, outfp);
	fputc(pals_i, outfp);
	fputc(pals_i >> 8, outfp);
	fputc(1, outfp);
	fputc(0, outfp);
	fwrite(pal, pals_i, 2, outfp);
	
	//Texture
	uint32_t tex_length = 12 + (((tex_width << 1) >> width_shift) * tex_height);
	fputc(tex_length, outfp);
	fputc(tex_length >> 8, outfp);
	fputc(tex_length >> 16, outfp);
	fputc(tex_length >> 24, outfp);
	fputc(tex_x, outfp);
	fputc(tex_x >> 8, outfp);
	fputc(tex_y, outfp);
	fputc(tex_y >> 8, outfp);
	fputc((tex_width >> width_shift), outfp);
	fputc((tex_width >> width_shift) >> 8, outfp);
	fputc(tex_height, outfp);
	fputc(tex_height >> 8, outfp);
	fwrite(tex, (tex_width << 1) >> width_shift, tex_height, outfp);
	free(tex);
	
	fclose(outfp);
	
	return 0;
}
