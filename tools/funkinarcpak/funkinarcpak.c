/*
 * funkinarcpak by Regan "CuckyDev" Green
 * Packs files into a single archive for the Friday Night Funkin' PSX port
*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void Write16(FILE *fp, uint16_t x)
{
	fputc(x, fp);
	fputc(x >> 8, fp);
}

void Write32(FILE *fp, uint32_t x)
{
	fputc(x, fp);
	fputc(x >> 8, fp);
	fputc(x >> 16, fp);
	fputc(x >> 24, fp);
}

int main(int argc, char *argv[])
{
	//Make sure the correct parameters have been given
	if (argc < 3)
	{
		printf("usage: funkinarcpak out ...\n");
		return 0;
	}
	
	//Open output
	FILE *out = fopen(argv[1], "wb");
	if (out == NULL)
	{
		printf("Failed to open %s\n", argv[1]);
		return 1;
	}
	
	//Allocate directory
	typedef struct
	{
		char name[12];
		uint32_t pos;
		uint32_t size;
		uint8_t *data;
	} Pkg_Directory;
	
	Pkg_Directory *dir = malloc(sizeof(Pkg_Directory) * (argc - 2));
	if (dir == NULL)
	{
		printf("Failed to allocate directory\n");
		return 1;
	}
	
	//Read files and fill directory
	Pkg_Directory *dirp = dir;
	for (int i = 2; i < argc; i++, dirp++)
	{
		//Open file
		FILE *in = fopen(argv[i], "rb");
		if (in == NULL)
		{
			printf("Failed to open %s\n", argv[i]);
			for (int j = 2; j < i; j++)
				free(dir[j - 2].data);
			free(dir);
			return 1;
		}
		
		//Read file
		fseek(in, 0, SEEK_END);
		dirp->size = ftell(in);
		dirp->data = malloc(dirp->size);
		if (dirp->data == NULL)
		{
			printf("Failed to allocate file buffer\n");
			fclose(in);
			for (int j = 2; j < i; j++)
				free(dir[j - 2].data);
			free(dir);
			return 1;
		}
		fseek(in, 0, SEEK_SET);
		fread(dirp->data, dirp->size, 1, in);
		fclose(in);
	}
	
	//Set directory positions
	dirp = dir;
	dirp->pos = 16 * (argc - 2);
	dirp++;
	
	for (int i = 3; i < argc; i++, dirp++)
		dirp->pos = (dirp[-1].pos + dirp[-1].size + 0xF) & ~0xF;
	
	//Write directory
	dirp = dir;
	for (int i = 2; i < argc; i++, dirp++)
	{
		//Cut path
		char *path = argv[i];
		
		char *cuts = path;
		cuts += strlen(cuts);
		while (cuts != (path - 1) && *cuts != '/' && *cuts != '\\') cuts--;
		cuts++;
		
		//Write directory
		if (strlen(cuts) > 12)
		{
			printf("Asset %s name is longer than 12 characters and will be truncated\n", cuts);
			fwrite(cuts, 12, 1, out);
		}
		else
		{
			fwrite(cuts, strlen(cuts), 1, out);
			for (size_t j = strlen(cuts); j < 12; j++)
				fputc('\0', out);
		}
		Write32(out, dirp->pos);
	}
	
	//Write file data
	dirp = dir;
	for (int i = 2; i < argc; i++, dirp++)
	{
		fseek(out, dirp->pos, SEEK_SET);
		fwrite(dirp->data, dirp->size, 1, out);
		free(dirp->data);
	}
	free(dir);
	fclose(out);
	
	return 0;
}
