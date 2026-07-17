// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// SDL sound backend replacement file.
// The obsolete Linux OSS (/dev/dsp), sndserver, signal timer, and original
// software mixer have been removed.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>

#include "doomdef.h"
#include "i_sound.h"
#include "i_system.h"
#include "w_wad.h"
#include "z_zone.h"

#undef KEY_ESCAPE
#undef KEY_ENTER
#undef KEY_TAB
#undef KEY_BACKSPACE
#undef KEY_PAUSE
#undef KEY_EQUAL
#undef KEY_MINUS

#undef KEY_F1
#undef KEY_F2
#undef KEY_F3
#undef KEY_F4
#undef KEY_F5
#undef KEY_F6
#undef KEY_F7
#undef KEY_F8
#undef KEY_F9
#undef KEY_F10
#undef KEY_F11
#undef KEY_F12
#include <raylib.h>

#define NUM_CHANNELS		32

typedef enum {
    SFX,
    MUSIC
} SoundType;

typedef struct {
    Sound sound;
    boolean active;
    float volume;
    SoundType type;
} SoundChannel;

static SoundChannel channels[NUM_CHANNELS];

void I_SetChannels(void)
{
    // idk if this is the right place for this
    for(int i = 0; i < NUM_CHANNELS; i++) {
        channels[i].sound = (Sound){0};
    }
}

static int I_FindOpenChannel(void) {
    for(int i = 0; i < NUM_CHANNELS; i++) {
        if(!channels[i].active) return i;
    }

    // replace channel 0
    channels[0].sound = (Sound){0};
    return 0;
}

void I_SetSfxVolume(int volume)
{
    snd_SfxVolume = volume;

    /* Apply the new volume to active SDL channels here. */
}

int I_GetSfxLumpNum(sfxinfo_t *sfx)
{
    char name[9];

    snprintf(name, sizeof(name), "ds%s", sfx->name);
    return W_GetNumForName(name);
}

static uint16_t ReadLE16(const uint8_t *p)
{
    return (uint16_t)p[0]
         | ((uint16_t)p[1] << 8);
}

static uint32_t ReadLE32(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

int I_StartSound(int id, int volume, int separation, int pitch, int priority)
{
    char lump_name[9];

    int channel_number = I_FindOpenChannel();
    // how the f*ck does SoundChannel *channel = &channel work??
    SoundChannel *channel = &channels[channel_number];
    if(id <= sfx_None || id >= NUMSFX) return -1;

    snprintf(
        lump_name,
        sizeof(lump_name),
        "DS%.6s",
        S_sfx[id].name
    );

    int lump_num = W_CheckNumForName(lump_name);
    if(lump_num < 0) {
        fprintf(stderr, "Missing sound lump: %s\n", lump_name);
        return -1;
    }

    int lump_length = W_LumpLength(lump_num);
    if(lump_length < 8) return -1;

    uint8_t *data = W_CacheLumpNum(lump_num, PU_STATIC); // sfx data from the lump

    uint16_t format = ReadLE16(data);
    uint16_t sample_rate = ReadLE16(data + 2);
    uint32_t sample_count = ReadLE32(data + 4);

    uint8_t *samples = data + 8;

    // validation
    if(format != 3) {
        fprintf(stderr, "Invalid Doom sound format: %u\n", (unsigned int)format);
        Z_ChangeTag(data, PU_CACHE);
        return -1;
    }

    if(sample_rate == 0) {
        Z_ChangeTag(data, PU_CACHE);
        return -1;
    }

    if(sample_count > (uint32_t)(lump_length - 8)) {
        fprintf(stderr, "Invalid sample count\n");
        Z_ChangeTag(data, PU_CACHE);
        return -1;
    }

    Wave wave = {
        .frameCount = sample_count,
        .sampleRate = sample_rate,
        .sampleSize = 8,
        .channels = 1,
        .data = data
    };

    channel->sound = LoadSoundFromWave(wave);
    channel->active = true;

    PlaySound(channel->sound);

    return channel_number;
}

void I_StopSound(int handle)
{
    StopSound(channels[handle].sound);
    UnloadSound(channels[handle].sound);

    channels[handle].sound = (Sound){0};
    channels[handle].active = false;
}

int I_SoundIsPlaying(int handle)
{
    boolean playing = IsSoundPlaying(channels[handle].sound);
    if(!playing) I_StopSound(handle);
    return (int)playing;
}

void I_UpdateSoundParams(int handle, int volume, int separation, int pitch)
{
    /* Update the active SDL channel identified by handle. */
    (void)handle;
    (void)volume;
    (void)separation;
    (void)pitch;
}

void I_UpdateSound(void)
{
    /*
     * Mix/fill audio here only if your SDL design uses game-thread-driven
     * mixing. Leave this empty when an SDL audio callback or SDL_AudioStream
     * owns continuous audio production.
     */
}

void I_SubmitSound(void)
{
    /*
     * Queue a completed mix buffer here only if your design requires it.
     * This function is unnecessary for callback/stream-driven output but is
     * retained because the Doom engine calls the platform sound API.
     */
}

void I_InitSound(void)
{
    InitAudioDevice();
}

void I_ShutdownSound(void)
{
    CloseAudioDevice();

}
/* ------------------------------------------------------------------------- */
/* Music API                                                                 */
/* ------------------------------------------------------------------------- */

void I_SetMusicVolume(int volume)
{
    snd_MusicVolume = volume;

    /* Apply the new volume to the SDL music path here. */
}

void I_InitMusic(void)
{
    /* Initialize the SDL/MIDI/MUS music backend here. */
}

void I_ShutdownMusic(void)
{
    /* Shut down and release music backend resources here. */
}

int I_RegisterSong(void *data)
{
    /* Register/convert the MUS or MIDI data and return a song handle. */
    (void)data;
    return 0;
}

void I_UnRegisterSong(int handle)
{
    /* Release the registered song identified by handle. */
    (void)handle;
}

void I_PlaySong(int handle, int looping)
{
    /* Start registered music through the new backend. */
    (void)handle;
    (void)looping;
}

void I_PauseSong(int handle)
{
    /* Pause the registered song. */
    (void)handle;
}

void I_ResumeSong(int handle)
{
    /* Resume the registered song. */
    (void)handle;
}

void I_StopSong(int handle)
{
    /* Stop the registered song. */
    (void)handle;
}

int I_QrySongPlaying(int handle)
{
    /* Return nonzero while the registered song is playing. */
    (void)handle;
    return 0;
}