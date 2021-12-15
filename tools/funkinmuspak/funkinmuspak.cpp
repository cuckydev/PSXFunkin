/*
 * funkinmuspak by Regan "CuckyDev" Green
 * Converts audio files to MUS files for the Friday Night Funkin' PSX port
*/

/*
  Uses ADPCM conversion by spicyjpeg
  (C) 2021 spicyjpeg
*/

#include <iostream>
#include <fstream>
#include <vector>

#include "adpcm.h"

struct MusChannel
{
	std::string path;
	float use_l, use_r;
};

struct MusData
{
	std::string path;
	float timestamp;
};

int main(int argc, char *argv[])
{
	//Check arguments
	if (argc < 3)
	{
		std::cout << "usage: funkinmuspak out_mus in_txt" << std::endl;
		return 0;
	}
	
	std::string path_mus = std::string(argv[1]);
	std::string path_txt = std::string(argv[2]);
	
	//Read txt file
	std::vector<MusChannel> mus_channels;
	
	std::ifstream stream_txt(path_txt);
	if (!stream_txt.is_open())
	{
		std::cout << "Failed to open txt " << path_txt << std::endl;
		return 1;
	}
	
	return 0;
}
