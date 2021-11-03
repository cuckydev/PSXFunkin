/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../io.h"

#include "../main.h"
#include "../mem.h"

//ISO directory
char *iso_dir = NULL;

//IO functions
void IO_Init(void)
{
	if (my_argc == 0)
	{
		//No directory was given
		iso_dir = NULL;
	}
	else
	{
		//Cut filename off the end of the directory
		const char *path = my_argv[0];
		size_t len = strlen(path);
		size_t cut = len - 1;
		for (; cut < len && path[cut] != '\\' && path[cut] != '/'; cut--) {;}
		
		//Allocate and copy to string
		if ((iso_dir = malloc(cut + 1 + 4 + 1)) == NULL)
		{
			iso_dir = NULL;
		}
		else
		{
			memcpy(iso_dir, path, cut + 1);
			iso_dir[cut + 1] = '\0';
			strcat(iso_dir, "ISO/");
		}
	}
}

void IO_Quit(void)
{
	free(iso_dir);
}

void IO_FindFile(CdlFILE *file, const char *path)
{
	//Empty path
	if (path == NULL || path[0] == '\0')
	{
		file->path[0] = '\0';
		return;
	}
	
	//Reformat path
	char *outp = file->path;
	const char *pathp = path + 1;
	for (size_t i = sizeof(file->path) - 1; i > 0; i--, outp++, pathp++)
	{
		if (*pathp == '\0' || *pathp == ';')
			break;
		if (*pathp == '\\')
			*outp = '/';
		else
			*outp = *pathp;
	}
	*outp = '\0';
}

void IO_SeekFile(CdlFILE *file)
{
	(void)file;
}

FILE *IO_OpenFile(CdlFILE *file)
{
	//Get joined path
	char *join;
	if (iso_dir != NULL)
	{
		join = malloc(strlen(iso_dir) + strlen(file->path) + 1);
		if (join == NULL)
		{
			sprintf(error_msg, "[IO_OpenFile] Failed to allocate joined path");
			ErrorLock();
			return NULL;
		}
		sprintf(join, "%s%s", iso_dir, file->path);
	}
	else
	{
		join = malloc(strlen(file->path) + 1);
		if (join == NULL)
		{
			sprintf(error_msg, "[IO_OpenFile] Failed to allocate joined path");
			ErrorLock();
			return NULL;
		}
		strcpy(join, file->path);
	}
	
	//Open file
	FILE *fp = fopen(join, "rb");
	free(join);
	if (fp == NULL)
	{
		sprintf(error_msg, "[IO_OpenFile] Failed to open \"%s\"", file->path);
		ErrorLock();
		return NULL;
	}
	
	return fp;
}

IO_Data IO_ReadFile(CdlFILE *file)
{
	//Open file
	FILE *fp = IO_OpenFile(file);
	if (fp == NULL)
		return NULL;
	
	//Allocate buffer
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	IO_Data data = Mem_Alloc(size);
	if (data == NULL)
	{
		fclose(fp);
		sprintf(error_msg, "[IO_ReadFile] Failed to allocate data buffer (size 0x%X)", (unsigned int)size);
		ErrorLock();
		return NULL;
	}
	fseek(fp, 0, SEEK_SET);
	
	//Read buffer
	if (fread(data, size, 1, fp) != 1)
	{
		fclose(fp);
		sprintf(error_msg, "[IO_ReadFile] Failed to allocate data buffer (size 0x%X)", (unsigned int)size);
		ErrorLock();
		return NULL;
	}
	fclose(fp);
	
	return data;
}

IO_Data IO_AsyncReadFile(CdlFILE *file)
{
	return IO_ReadFile(file);
}

IO_Data IO_Read(const char *path)
{
	CdlFILE file;
	IO_FindFile(&file, path);
	return IO_ReadFile(&file);
}

IO_Data IO_AsyncRead(const char *path)
{
	CdlFILE file;
	IO_FindFile(&file, path);
	return IO_AsyncReadFile(&file);
}

boolean IO_IsSeeking(void)
{
	return false;
}

boolean IO_IsReading(void)
{
	return false;
}
