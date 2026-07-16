//-----------------------------------------------------------------------------
//
// i_sound_stub.c
//
// Cross-platform no-audio backend for LinuxDOOM.
// This file satisfies the engine's sound and music interface without using
// OSS, /dev/dsp, ioctl, timers, or any platform-specific audio APIs.
//
//-----------------------------------------------------------------------------

#include <stddef.h>

#include "doomdef.h"
#include "i_sound.h"

char *sndserver_filename = "";

void I_SetChannels(void)
{
}

void I_SetSfxVolume(int volume)
{
    (void) volume;
}

void I_SetMusicVolume(int volume)
{
    (void) volume;
}

int I_GetSfxLumpNum(sfxinfo_t *sfx)
{
    (void) sfx;
    return 0;
}

int I_StartSound(
    int id,
    int volume,
    int separation,
    int pitch,
    int priority)
{
    (void) volume;
    (void) separation;
    (void) pitch;
    (void) priority;

    // Returning the sound ID gives the caller a stable dummy handle.
    return id;
}

void I_StopSound(int handle)
{
    (void) handle;
}

int I_SoundIsPlaying(int handle)
{
    (void) handle;
    return 0;
}

void I_UpdateSound(void)
{
}

void I_SubmitSound(void)
{
}

void I_UpdateSoundParams(
    int handle,
    int volume,
    int separation,
    int pitch)
{
    (void) handle;
    (void) volume;
    (void) separation;
    (void) pitch;
}

void I_ShutdownSound(void)
{
}

void I_InitSound(void)
{
}

void I_InitMusic(void)
{
}

void I_ShutdownMusic(void)
{
}

void I_PlaySong(int handle, int looping)
{
    (void) handle;
    (void) looping;
}

void I_PauseSong(int handle)
{
    (void) handle;
}

void I_ResumeSong(int handle)
{
    (void) handle;
}

void I_StopSong(int handle)
{
    (void) handle;
}

void I_UnRegisterSong(int handle)
{
    (void) handle;
}

int I_RegisterSong(void *data)
{
    (void) data;

    // Return a nonzero dummy song handle.
    return 1;
}

int I_QrySongPlaying(int handle)
{
    (void) handle;
    return 0;
}