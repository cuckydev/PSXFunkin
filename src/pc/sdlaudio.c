/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../audio.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../io.h"
#include "../main.h"

#include "SDL.h"

//We really really don't care if dr_mp3 has unused functions
#ifdef __GNUC__
 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#define DRMP3_API static
#include "dr_mp3.h"

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MA_NO_DEVICE_IO
#define MA_NO_THREADING
#define MA_API static
#include "miniaudio.h"

#ifdef __GNUC__
 #pragma GCC diagnostic pop
#endif

//XA state
#define XA_STATE_PLAYING (1 << 0)
#define XA_STATE_LOOPS   (1 << 1)

static XA_Track xa_track;
static u8 xa_channel;

static u8 xa_state;

//SDL audio
static SDL_AudioDeviceID audio_device;

static size_t bytes_per_frame = 4; //stereo s16

static Uint32 xa_lasttime, xa_interptime, xa_interpstart;

//MP3 decode
typedef struct
{
	boolean playing;
	unsigned char *data, *datap, *datae;
} MP3Decode;

MP3Decode xa_mp3[2];

extern IO_Data IO_ReadFile2(CdlFILE *file, size_t *size);

static boolean MP3Decode_Decode(MP3Decode *this, CdlFILE *file)
{
	//Open file and read contents
	size_t size;
	IO_Data data = IO_ReadFile2(file, &size);
	if (data == NULL)
		return true;
	
	//Prepare dr_mp3 decoding
	drmp3 drmp3_instance;
	if (!drmp3_init_memory(&drmp3_instance, data, size, NULL))
	{
		sprintf(error_msg, "[MP3Decode_Decode] Failed to initialize dr_mp3 instance");
		ErrorLock();
		return true;
	}
	
	drmp3_uint64 decoded_frames = drmp3_get_pcm_frame_count(&drmp3_instance);
	drmp3_uint32 decoded_channels = drmp3_instance.channels;
	drmp3_uint32 decoded_samplerate = drmp3_instance.sampleRate;
	
	//Decode entire file to buffer
	short *decoded = malloc(decoded_frames * decoded_channels * sizeof(short));
	
	drmp3_read_pcm_frames_s16(&drmp3_instance, decoded_frames, decoded);
	drmp3_uninit(&drmp3_instance);
	free(data);

	//Convert buffer to output format
	ma_uint64 output_frames = ma_convert_frames(NULL, 0, ma_format_s16, 2, 44100, decoded, decoded_frames, ma_format_s16, decoded_channels, decoded_samplerate);

	this->data = malloc(output_frames * bytes_per_frame);

	if (this->data == NULL)
	{
		sprintf(error_msg, "[MP3Decode_Decode] Failed to allocate converted audio buffer");
		ErrorLock();
		return true;
	}

	output_frames = ma_convert_frames(this->data, output_frames, ma_format_s16, 2, 44100, decoded, decoded_frames, ma_format_s16, decoded_channels, decoded_samplerate);
	free(decoded);
	
	this->datap = this->data;
	this->datae = this->data + (output_frames * bytes_per_frame);
	
	return false;
}

static size_t MP3Decode_Copy(MP3Decode *this, unsigned char *stream, size_t bytes_to_do)
{
	//Make sure data exists
	if (this->data == NULL || this->datap == NULL || this->datae == NULL)
	{
		memset(stream, 0, bytes_to_do);
		return bytes_to_do;
	}

	//Calculate bytes left
	size_t bytes_done = bytes_to_do;
	size_t bytes_left = this->datae - this->datap;
	if (bytes_done > bytes_left)
		bytes_done = bytes_left;

	//Copy data
	memcpy(stream, this->datap, bytes_done);

	//Advance pointer
	this->datap += bytes_done;

	return bytes_done;
}

static void MP3Decode_Skip(MP3Decode *this, size_t bytes_to_skip)
{
	//Make sure data exists
	if (this->data == NULL || this->datap == NULL || this->datae == NULL)
		return;
	
	//Skip
	this->datap += bytes_to_skip;
	if (this->datap >= this->datae)
		this->datap = this->datae;
}

//Audio callback
static void Audio_Callback(void *userdata, Uint8 *output_buffer_void, int bytes_to_do)
{
	//Copy XA
	if (xa_state & XA_STATE_PLAYING)
	{
		//Update timing state
		xa_interptime = xa_lasttime;
		xa_interpstart = SDL_GetTicks();
		xa_lasttime = (Sint64)(xa_mp3[xa_channel].datap - xa_mp3[xa_channel].data) / bytes_per_frame * 1000 / 44100;
		
		//Copy MP3s into stream
		unsigned char *output_buffer = output_buffer_void;
		size_t bytes_remaining = bytes_to_do;
		while (bytes_remaining != 0)
		{
			size_t bytes_done = MP3Decode_Copy(&xa_mp3[xa_channel], output_buffer, bytes_remaining);
			MP3Decode_Skip(&xa_mp3[xa_channel ^ 1], bytes_done);
			
			output_buffer += bytes_done;
			bytes_remaining -= bytes_done;
			
			//Check if songs ended
			if ((xa_mp3[0].data == NULL || xa_mp3[0].datap >= xa_mp3[0].datae) && (xa_mp3[1].data == NULL || xa_mp3[1].datap >= xa_mp3[1].datae))
			{
				if (xa_state & XA_STATE_LOOPS)
				{
					//Reset pointers
					xa_mp3[0].datap = xa_mp3[0].data;
					xa_mp3[1].datap = xa_mp3[1].data;
				}
				else
				{
					//Stop playing
					xa_state &= ~XA_STATE_PLAYING;
					memset(output_buffer, 0, bytes_remaining);
					break;
				}
			}
		}
	}
	else
	{
		//Clear stream
		memset(output_buffer_void, 0, bytes_to_do);
	}
}

//XA files and tracks
static CdlFILE xa_files[XA_TrackMax];

#include "../audio_def.h"

//Audio functions
void Audio_Init(void)
{
	//Get file positions
	CdlFILE *filep = xa_files;
	for (const XA_Mp3 *mp3 = xa_mp3s; mp3->name != NULL; mp3++, filep++)
	{
		char apath[64];
		if (mp3->vocal)
		{
			sprintf(apath, "\\MUSIC\\%sv.mp3;1", mp3->name);
			IO_FindFile(filep, apath);
		}
		else
		{
			sprintf(apath, "\\MUSIC\\%s.mp3;1", mp3->name);
			IO_FindFile(filep, apath);
		}
	}
	
	//Initialize XA state
	xa_track = -1;
	xa_channel = 0;
	
	xa_state = 0;
	
	xa_mp3[0].data = NULL;
	xa_mp3[1].data = NULL;
	
	//Open audio device
	SDL_AudioSpec want, have;
	
	SDL_memset(&want, 0, sizeof(want));
	want.freq = 44100;
	want.format = AUDIO_S16;
	want.channels = 2;
	want.samples = 256;
	want.callback = Audio_Callback;
	
	if ((audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0)) == 0)
	{
		sprintf(error_msg, "[Audio_Init] Failed to open audio device: %s", SDL_GetError());
		ErrorLock();
	}
	
	printf("%d\n", have.samples);
	
	SDL_PauseAudioDevice(audio_device, 0);
}

void Audio_Quit(void)
{
	//Destroy audio device
	SDL_CloseAudioDevice(audio_device);
	
	//Free mp3s
	free(xa_mp3[0].data);
	free(xa_mp3[1].data);
}

void Audio_PlayXA_Track(XA_Track track, u8 volume, u8 channel, boolean loop)
{
	//Ensure track is loaded
	Audio_SeekXA_Track(track);
	
	//Lock mutex during state modification
	SDL_LockAudioDevice(audio_device);
	xa_state = XA_STATE_PLAYING | (loop ? XA_STATE_LOOPS : 0);
	SDL_UnlockAudioDevice(audio_device);
}

void Audio_SeekXA_Track(XA_Track track)
{
	//Lock mutex during state modification
	SDL_LockAudioDevice(audio_device);
	
	//Reset XA state
	xa_state = 0;
	xa_channel = 0;
	xa_lasttime = xa_interptime = 0;
	xa_interpstart = SDL_GetTicks();
	
	//Read file if different track
	if (track != xa_track)
	{
		//Free previous track
		free(xa_mp3[0].data);
		free(xa_mp3[1].data);
		
		//Read new track
		if (xa_mp3s[track].vocal)
		{
			char *path = xa_files[track].path;
			path[strlen(path) - 5] = 'v';
			MP3Decode_Decode(&xa_mp3[0], &xa_files[track]);
			path[strlen(path) - 5] = 'i';
			MP3Decode_Decode(&xa_mp3[1], &xa_files[track]);
		}
		else
		{
			MP3Decode_Decode(&xa_mp3[0], &xa_files[track]);
			xa_mp3[1].data = NULL;
		}
		
		//Remember
		xa_track = track;
	}
	
	//Unlock mutex
	SDL_UnlockAudioDevice(audio_device);
}

void Audio_PauseXA(void)
{
	//Lock mutex during state modification
	SDL_LockAudioDevice(audio_device);
	xa_state &= ~XA_STATE_PLAYING;
	SDL_UnlockAudioDevice(audio_device);
}

void Audio_StopXA(void)
{
	//Lock mutex during state modification
	SDL_LockAudioDevice(audio_device);
	
	//Set XA state
	xa_track = -1;
	xa_state = 0;
	xa_channel = 0;
	xa_lasttime = xa_interptime = 0;
	xa_interpstart = SDL_GetTicks();
	
	//Free previous track
	free(xa_mp3[0].data); xa_mp3[0].data = NULL;
	free(xa_mp3[1].data); xa_mp3[1].data = NULL;
	
	//Unlock mutex
	SDL_UnlockAudioDevice(audio_device);
}

void Audio_ChannelXA(u8 channel)
{
	//Lock mutex during state modification
	SDL_LockAudioDevice(audio_device);
	if (xa_mp3s[xa_track].vocal)
		xa_channel = channel & 1;
	SDL_UnlockAudioDevice(audio_device);
}

s32 Audio_TellXA_Sector(void)
{
	return (s64)Audio_TellXA_Milli() * 75 / 1000; //trolled
}

s32 Audio_TellXA_Milli(void)
{
	//Lock mutex during state check
	SDL_LockAudioDevice(audio_device);
	
	s32 pos;
	if (xa_state & XA_STATE_PLAYING)
	{
		double xa_timesec = xa_interptime + (double)(SDL_GetTicks() - xa_interpstart);
		pos = (s32)xa_timesec;
	}
	else
	{
		pos = 0;
	}
	
	//Unlock mutex
	SDL_UnlockAudioDevice(audio_device);
	return pos;
}

boolean Audio_PlayingXA(void)
{
	//Lock mutex during state check
	SDL_LockAudioDevice(audio_device);
	
	boolean playing = (xa_state & XA_STATE_PLAYING) != 0;
	
	//Unlock mutex
	SDL_UnlockAudioDevice(audio_device);
	return playing;
}

void Audio_WaitPlayXA(void)
{
	
}

void Audio_ProcessXA(void)
{
	
}
