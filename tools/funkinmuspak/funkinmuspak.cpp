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
#include <unordered_map>
#include <string>
#include <iomanip>
#include <string.h>

#include "adpcm.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_ENCODING
#define MA_NO_DEVICE_IO
#define MA_HAS_VORBIS
#define MA_API static
#include "miniaudio.h"

#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

//Audio constants
#define SAMPLE_RATE 44100

#define CHUNK_SECTORS 13

#define SECTOR_BLOCKS (2048 / 16)
#define CHUNK_BLOCKS (SECTOR_BLOCKS * CHUNK_SECTORS)

#define SECTOR_SAMPLES (28 * SECTOR_BLOCKS)
#define CHUNK_SAMPLES (SECTOR_SAMPLES * CHUNK_SECTORS)

//Mus structures
struct MusAudio
{
	//Audio data
	std::vector<int16_t> data;
	size_t chunks = 0;
};

struct MusChannel
{
	//Descriptor
	std::string path;
	float use_l = 0.0f, use_r = 0.0f;
	
	//Audio data
	MusAudio *audio = nullptr;
	std::vector<uint8_t> adpcm;
};

struct MusData
{
	//Descriptor
	std::string path;
	float timestamp = 0.0f;
	
	//File data
};

//Entry point
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
	
	std::string path_base;
	auto path_base_cut = path_txt.find_last_of("/\\");
	if (path_base_cut != std::string::npos)
		path_base = path_txt.substr(0, path_base_cut + 1);
	
	//Read txt file
	std::unordered_map<std::string, MusAudio> mus_audio;
	std::vector<MusChannel> mus_channels;
	
	std::ifstream stream_txt(path_txt);
	if (!stream_txt.is_open())
	{
		std::cout << "Failed to open txt " << path_txt << std::endl;
		return 1;
	}
	
	//Read channels
	size_t num_channels;
	stream_txt >> num_channels;
	std::cout << num_channels << " channels:" << std::endl;
	
	for (size_t i = 0; i < num_channels; i++)
	{
		//Read descriptor
		MusChannel channel;
		stream_txt >> std::quoted(channel.path) >> channel.use_l >> channel.use_r;
		std::cout << channel.path << ", " << channel.use_l << ", " << channel.use_r << std::endl;
		
		//Read audio
		auto audio_find = mus_audio.find(channel.path);
		if (audio_find != mus_audio.end())
		{
			//Use already loaded audio
			channel.audio = &audio_find->second;
		}
		else
		{
			//Load audio
			std::string path = path_base + channel.path;
			MusAudio audio;
			
			//Create miniaudio decoder
			ma_decoder decoder;
			ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_s16, 2, SAMPLE_RATE);
			
			if (ma_decoder_init_file(path.c_str(), &decoder_config, &decoder) != MA_SUCCESS)
			{
				std::cout << "Failed to open audio " << path << std::endl;
				return 1;
			}
			
			//Read PCM data
			int16_t data[CHUNK_SAMPLES * 2];
			
			while (1)
			{
				//Read a chunk's worth of data
				memset(data, 0, sizeof(data));
				if (ma_decoder_read_pcm_frames(&decoder, data, CHUNK_SAMPLES) == 0)
					break;
				audio.data.insert(audio.data.end(), data, data + (sizeof(data) / sizeof(int16_t)));
				audio.chunks += 1;
			}
			
			//Delete miniaudio decoder
			ma_decoder_uninit(&decoder);
			
			//Push audio to audio map
			mus_audio[channel.path] = audio;
			channel.audio = &mus_audio[channel.path];
		}
		
		//Encode audio to ADPCM
		std::vector<int16_t> mix_buffer(channel.audio->chunks * CHUNK_SAMPLES);
		for (size_t i = 0; i < mix_buffer.size(); i++)
			mix_buffer[i] = (int16_t)((float)channel.audio->data[(i << 1)] * channel.use_l + (float)channel.audio->data[(i << 1) | 1] * channel.use_r);
		
		channel.adpcm.resize(channel.audio->chunks * CHUNK_BLOCKS * 16);
		spu::encodeSound(mix_buffer.data(), channel.adpcm.data(), mix_buffer.size(), mix_buffer.size());
		
		//Push channel to channel list
		mus_channels.push_back(channel);
	}
	
	//Read data
	
	//Close txt file
	stream_txt.close();
	
	//Write mus file
	std::ofstream stream_mus(path_mus, std::ios::binary);
	if (!stream_mus.is_open())
	{
		std::cout << "Failed to open mus " << path_mus << std::endl;
		return 1;
	}
	
	//Write meta header
	uint8_t header[2048] = {};
	header[0] = mus_channels.size();
	
	stream_mus.write((const char*)header, 2048);
	
	//Write sector by sector
	size_t chunks = 1;
	for (auto &j : mus_audio)
		if (j.second.chunks > chunks)
			chunks = j.second.chunks;
	
	for (size_t i = 0; i < chunks; i++)
	{
		//Write channels
		for (auto &j : mus_channels)
		{
			//Write audio data
			if (i < j.audio->chunks)
			{
				//Convert and write blocks
				for (size_t k = 0; k < CHUNK_BLOCKS; k++)
				{
					uint8_t block[16] = {};
					memcpy(block, j.adpcm.data() + ((i * CHUNK_BLOCKS + k) * 16), 16);
					if (k == (CHUNK_BLOCKS - 1))
						block[1] = 0x03;
					stream_mus.write((const char*)block, 16);
				}
			}
			else
			{
				//Write silent blocks
				for (size_t k = 0; k < CHUNK_BLOCKS; k++)
				{
					uint8_t block[16] = {0x0B};
					if (k == (CHUNK_BLOCKS - 1))
						block[1] = 0x03;
					stream_mus.write((const char*)block, 16);
				}
			}
		}
	}
	
	//Close mus file
	stream_mus.close();
	
	return 0;
}
