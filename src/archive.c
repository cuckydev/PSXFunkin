#include "archive.h"

//Archive structure
typedef struct
{
	char path[12];
	u32 pos;
} ArchiveFile;

//Archive functions
IO_Data Archive_Find(IO_Data arc, const char *path)
{
	//Check against all archive files
	for (const ArchiveFile *file = (const ArchiveFile*)arc; file->path[0] != '\0'; file++)
	{
		if (strncmp(file->path, path, 12))
			continue;
		return (IO_Data)((u8*)arc + file->pos);
	}
	return NULL;
}
