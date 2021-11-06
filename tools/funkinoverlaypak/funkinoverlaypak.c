/*
 * funkinoverlaypak by Regan "CuckyDev" Green
 * Packs overlay and temporary data together for the Friday Night Funkin' PSX port
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

int main(int argc, char *argv[])
{
	//Read parameters
	if (argc < 3)
	{
		printf("usage: funkinoverlaypak out.exe overlay.in ...\n");
		return 0;
	}
	FILE *fp;
	
	//Read overlay
	fp = fopen(argv[2], "rb");
	if (fp == NULL)
	{
		printf("Failed to open %s\n", argv[2]);
		return 1;
	}
	
	fseek(fp, 0, SEEK_END);
	
	size_t overlay_size = ftell(fp);
	size_t overlay_sects = (overlay_size + 0x7FF) >> 11;
	unsigned char *overlay_data = malloc(overlay_sects << 11);
	if (overlay_data == NULL)
	{
		printf("Failed to allocate overlay data buffer\n");
		return 1;
	}
	
	rewind(fp);
	memset(overlay_data, 0, overlay_sects << 11);
	fread(overlay_data, overlay_size, 1, fp);
	fclose(fp);
	
	//Modify overlay data
	overlay_data[0] = overlay_sects;
	overlay_data[1] = overlay_sects >> 8;
	
	//Allocate directory
	typedef struct
	{
		uint16_t size;
		uint8_t *data;
	} File;
	
	size_t files = argc - 3;
	char **filev = (&argv[0]) + 3;
	
	File *file = malloc(sizeof(File) * files);
	if (file == NULL)
	{
		printf("Failed to allocate files\n");
		free(overlay_data);
		return 1;
	}
	
	//Read files
	File *filep = file;
	for (size_t i = 0; i < files; i++, filep++)
	{
		//Read file
		fp = fopen(filev[i], "rb");
		if (fp == NULL)
		{
			printf("Failed to open %s\n", filev[i]);
			for (size_t j = 0; j < i; j++)
				free(file[j].data);
			free(file);
			free(overlay_data);
			return 1;
		}
		
		fseek(fp, 0, SEEK_END);
		
		size_t data_size = ftell(fp);
		filep->size = (data_size + 0x7FF) >> 11;
		if ((filep->data = malloc(filep->size << 11)) == NULL)
		{
			printf("Failed to allocate data buffer\n");
			for (size_t j = 0; j < i; j++)
				free(file[j].data);
			free(file);
			free(overlay_data);
			fclose(fp);
			return 1;
		}
		memset(filep->data, 0, filep->size << 11);
		
		rewind(fp);
		fread(filep->data, data_size, 1, fp);
		fclose(fp);
	}
	
	//Write file
	fp = fopen(argv[1], "wb");
	if (fp == NULL)
	{
		printf("Failed to open %s\n", argv[1]);
		for (size_t j = 0; j < files; j++)
			free(file[j].data);
		free(file);
		free(overlay_data);
		return 1;
	}
	
	fwrite(overlay_data, overlay_sects << 11, 1, fp);
	
	filep = file;
	for (size_t i = 0; i < files; i++, filep++)
	{
		fputc(filep->size, fp);
		fputc(filep->size >> 8, fp);
	}
	fseek(fp, 2048 - (files * 2), SEEK_CUR);
	
	filep = file;
	for (size_t i = 0; i < files; i++, filep++)
		fwrite(filep->data, filep->size << 11, 1, fp);
	
	fclose(fp);
	
	for (size_t j = 0; j < files; j++)
		free(file[j].data);
	free(file);
	free(overlay_data);
	
	return 0;
}
