/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Sdl/SdlSound.c,v $
**
** $Revision: 1.6 $
**
** $Date: 2008-03-30 18:38:45 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/
#include "ArchSound.h"
#include <SDL.h>
#include <stdlib.h>

typedef struct SdlSound {
    Mixer* mixer;
    int started;
    UInt32 readPtr;
    UInt32 writePtr;
    UInt32 bytesPerSample;
    UInt32 bufferMask;
    UInt32 bufferSize;
    UInt32 skipCount;
    UInt8* buffer;
} SdlSound;


void printStatus(SDL_AudioDeviceID dev)
{
    switch (SDL_GetAudioDeviceStatus(dev))
    {
        case SDL_AUDIO_STOPPED: printf("stopped\n"); break;
        case SDL_AUDIO_PLAYING: printf("playing\n"); break;
        case SDL_AUDIO_PAUSED: printf("paused\n"); break;
        default: printf("???"); break;
    }
}

/*
// device starts paused
SDL_AudioDeviceID dev;
dev = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
if (dev != 0)
{
     printStatus(dev);  // prints "paused"
     SDL_PauseAudioDevice(dev, 0);
     printStatus(dev);  // prints "playing"
     SDL_PauseAudioDevice(dev, 1);
     printStatus(dev);  // prints "paused"
     SDL_CloseAudioDevice(dev);
     printStatus(dev);  // prints "stopped"
}
*/ 

SDL_AudioDeviceID dev;
SdlSound sdlSound;
int oldLen = 0;
void soundCallback(void* userdata, Uint8* stream, int length)
{
    UInt32 avail = (sdlSound.readPtr - sdlSound.writePtr);// & sdlSound.bufferMask;
oldLen = length;
    if ((UInt32)length > avail) {
        sdlSound.readPtr = (sdlSound.readPtr - sdlSound.bufferSize / 2) & sdlSound.bufferMask;
        memset((UInt8*)stream + avail, 0, length - avail);
        length = avail;
    }

    memcpy(stream, sdlSound.buffer + sdlSound.readPtr, length);
    sdlSound.readPtr = (sdlSound.readPtr + length) & sdlSound.bufferMask;
}

static Int32 soundWrite(SdlSound* dummy, Int16 *buffer, UInt32 count)
{
    UInt32 avail;

    if (!sdlSound.started) {
        return 0;
    }

    count *= sdlSound.bytesPerSample;

    if (sdlSound.skipCount > 0) {
        if (count <= sdlSound.skipCount) {
            sdlSound.skipCount -= count;
            return 0;
        }
        count -= sdlSound.skipCount;
        sdlSound.skipCount = 0;
    }

    SDL_LockAudio();

    avail = (sdlSound.writePtr - sdlSound.readPtr) & sdlSound.bufferMask;
    if (count < avail && 0) {
        sdlSound.skipCount = sdlSound.bufferSize / 2;
    }
    else {
        if (sdlSound.writePtr + count > sdlSound.bufferSize) {
            UInt32 count1 = sdlSound.bufferSize - sdlSound.writePtr;
            UInt32 count2 = count - count1;
            memcpy(sdlSound.buffer + sdlSound.writePtr, buffer, count1);
            memcpy(sdlSound.buffer, buffer, count2);
            sdlSound.writePtr = count2;
        }
        else {
            memcpy(sdlSound.buffer + sdlSound.writePtr, buffer, count);
            sdlSound.writePtr += count;
        }
    }

    SDL_UnlockAudio();
    return 0;
}

void archSoundCreate(Mixer* mixer, UInt32 sampleRate, UInt32 bufferSize, Int16 channels) 
{
	SDL_AudioSpec desired;
	SDL_AudioSpec audioSpec;
	UInt16 samples = channels;

    memset(&sdlSound, 0, sizeof(sdlSound));

    bufferSize = bufferSize * sampleRate / 1000 * sizeof(Int16) / 4;

    while (samples < bufferSize) samples *= 2;

	desired.freq     = sampleRate;
	desired.samples  = samples;
	desired.channels = (UInt8)channels;
#ifdef LSB_FIRST
	desired.format   = AUDIO_S16LSB;
#else
        desired.format   = AUDIO_S16MSB;
#endif
	desired.callback = soundCallback;
	desired.userdata = NULL;
    
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		fprintf(stderr,"Failed to run SDL_InitSubSystem\n");
        return;
    }

	/*if (SDL_OpenAudio(&desired, &audioSpec) != 0) {
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		fprintf(stderr,"SDL_OpenAudio failed with %s\n",SDL_GetError());
        return;
    }*/
	//dev = SDL_OpenAudioDevice(NULL, 0, &desired, &audioSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	dev = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0,0), 0, &desired, &audioSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	if (dev == 0) {
    		SDL_Log("Failed to open audio: %s", SDL_GetError());
	} else {
    		if (audioSpec.format != desired.format) { /* we let this one thing change. */
        	SDL_Log("We didn't get Float32 audio format.");
    	}
    	SDL_PauseAudioDevice(dev, 0); /* start audio playing. */
	}

	
	printf ("freq:%d(%d)\n", desired.freq, audioSpec.freq);
	printf ("samples:%d(%d)\n", desired.samples, audioSpec.samples);
	printf ("channels:%d(%d)\n", desired.channels, audioSpec.channels);
	printf ("format:%d(%d)\n", desired.format, audioSpec.format);
	printf ("size:%d(%d)\n", desired.size, audioSpec.size);
	
    sdlSound.bufferSize = 5;
    while (sdlSound.bufferSize < 4 * audioSpec.size) sdlSound.bufferSize *= 2;
	sdlSound.bufferSize = audioSpec.size * 4;
	printf ("size:%d\n", sdlSound.bufferSize);
    sdlSound.bufferMask = sdlSound.bufferSize - 1;
    sdlSound.buffer = (UInt8*)calloc(1, sdlSound.bufferSize);
    sdlSound.started = 1;
    sdlSound.mixer = mixer;
    sdlSound.bytesPerSample = audioSpec.format == AUDIO_U8 || audioSpec.format == AUDIO_S8 ? 1 : 2;
    
    mixerSetStereo(mixer, channels == 2);
    mixerSetWriteCallback(mixer, soundWrite, NULL, audioSpec.size / sdlSound.bytesPerSample);
    
	SDL_PauseAudio(0);
	fprintf(stderr,"Audio device %lu status: ",dev);
	printStatus(dev);
	
}

void archSoundDestroy(void) 
{
    if (sdlSound.started) {
        mixerSetWriteCallback(sdlSound.mixer, NULL, NULL, 0);
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    sdlSound.started = 0;

}
void archSoundResume(void) 
{
	SDL_PauseAudioDevice(dev,0);
}

void archSoundSuspend(void) 
{
	SDL_PauseAudioDevice(dev,1);
}
