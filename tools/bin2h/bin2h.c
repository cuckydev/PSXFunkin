/* bin2h - converts binary files to C header files */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int result = 1;

	if (argc > 2)
	{
		FILE *in_file = fopen(argv[1], "rb");
		FILE *out_file = fopen(argv[2], "w");

		if (in_file == NULL)
		{
			printf("Couldn't open '%s'\n", argv[1]);
		}
		else if (out_file == NULL)
		{
			printf("Couldn't open '%s'\n", argv[2]);
		}
		else
		{
			long in_file_size;
			unsigned char *in_file_buffer;
			unsigned char *in_file_pointer;
			long i;

			fseek(in_file, 0, SEEK_END);
			in_file_size = ftell(in_file);
			rewind(in_file);
			in_file_buffer = malloc(in_file_size);
			if (fread(in_file_buffer, 1, in_file_size, in_file) < in_file_size)
			{
				printf("Couldn't read '%s'\n", argv[1]);
				fclose(in_file);
				return -1;
			}
			fclose(in_file);
			in_file_pointer = in_file_buffer;

			setvbuf(out_file, NULL, _IOFBF, 0x10000);

			for (i = 0; i < in_file_size - 1; ++i)
			{
				if (i % 64 == 64-1)
					fprintf(out_file, "%d,\n", *in_file_pointer++);
				else
					fprintf(out_file, "%d,", *in_file_pointer++);
			}

			fprintf(out_file, "%d\n", *in_file_pointer++);

			fclose(out_file);
			free(in_file_buffer);
			result = 0;
		}
	}

	return result;
}
