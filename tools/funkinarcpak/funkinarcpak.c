/*
 * funkinarcpak by Regan "CuckyDev" Green
 * Packs files into a single archive for the Friday Night Funkin' PSX port
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void Write32(FILE *fp, uint32_t x)
{
	fputc(x, fp);
	fputc(x >> 8, fp);
	fputc(x >> 16, fp);
	fputc(x >> 24, fp);
}

int main(int argc, char *argv[])
{
	//Check parameters
	if (argc < 4)
	{
		puts("usage: funkinarcpak out ...");
		return 0;
	}
	
	//Read archive files
	typedef struct
	{
		char path[12];
		size_t pos, size;
		void *data;
	} File;//files[16];
	
	int filec = argc - 2;
	File *files = malloc(sizeof(File) * (filec + 1));
	
	size_t file_pos = (filec + 1) * 16;
	for (int i = 0; i < filec + 1; i++)
	{
		memset(files[i].path, '\0', 14);
		
		if (i >= filec)
		{
			//File not given
			files[i].data = NULL;
			files[i].pos = 0;
		}
		else
		{
			//Use given path
			char *path = argv[2 + i];
			strncpy(files[i].path, path, 12);
			
			//Open file
			FILE *in = fopen(path, "rb");
			if (in == NULL)
			{
				printf("Failed to open %s\n", path);
				for (int v = 0; v < i; v++)
					free(files[v].data);
				return 1;
			}
			
			//Get file size
			fseek(in, 0, SEEK_END);
			size_t size = files[i].size = ftell(in);
			fseek(in, 0, SEEK_SET);
			
			//Read file
			if ((files[i].data = malloc((size + 0xF) & ~0xF)) == NULL)
			{
				puts("Failed to allocate file buffer");
				for (int v = 0; v < i; v++)
					free(files[v].data);
				fclose(in);
				return 1;
			}
			memset(files[i].data, 0, size);
			if (fread(files[i].data, size, 1, in) != 1)
			{
				printf("Failed to read %s\n", path);
				for (int v = 0; v < i; v++)
					free(files[v].data);
				fclose(in);
			}
			fclose(in);
			
			//Set file pos
			files[i].pos = file_pos;
			file_pos = (file_pos + size + 0xF) & ~0xF;
		}
	}
	
	//Open archive file
	FILE *fp = fopen(argv[1], "wb");
	if (fp == NULL)
	{
		printf("Failed to open %s\n", argv[1]);
		return 1;
	}
	
	//Write header
	for (int i = 0; i < filec + 1; i++)
	{
		fwrite(files[i].path, 12, 1, fp);
		Write32(fp, files[i].pos);
	}
	
	//Write data
	for (int i = 0; i < filec + 1; i++)
	{
		if (files[i].data == NULL)
			break;
		fseek(fp, files[i].pos, SEEK_SET);
		fwrite(files[i].data, files[i].size, 1, fp);
	}
	
	fclose(fp);
	return 0;
}
