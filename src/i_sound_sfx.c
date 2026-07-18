#include "i_sound.h"
#include "z_zone.h"
#include "w_wad.h"
#include "i_system.h"

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

// heres a riddle for you:
// if i wrote the code to play sfx but never call it
// where do the sound effects come from?
// answer at the bottom of the file

#define MAX_CHANNELS 32

typedef struct {
    Sound sound;
    boolean active;
} SoundChannel;

static SoundChannel channels[MAX_CHANNELS];
int sfx_volume = 0;

/// @brief Sets up the SFX channes
void I_SetChannels(void) {
    for(int i = 0; i < MAX_CHANNELS; i++) {
        channels[i].sound = (Sound){0};
    }
}

/// @brief Finds an available channel
/// @return Handle/index to open channel
static int I_FindOpenChannel(void) {
    for(int i = 0; i < MAX_CHANNELS; i++) {
        if(!channels[i].active) return i;
    }

    channels[0].sound = (Sound){0};
    return 0;
}

/// @brief Sets a SFX volume
/// @param volume volume
void I_SetSfxVolume(int volume) {
    sfx_volume = volume;
}

/// @brief Returns a LUMP number for a SFX
/// @param sfx SFX
/// @return number
int I_GetSfxLumpNum(sfxinfo_t *sfx) {
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

/// @brief Plays a SFX
/// @param id SFX ID
/// @param vol Volume
/// @param sep Seperation
/// @param pitch Pitch
/// @param priority Priority
/// @return Channel number
int I_StartSound(int id, int vol, int sep, int pitch, int priority) {
    int channel_number = I_FindOpenChannel();

    SoundChannel *channel = &channels[channel_number];
    if(id <= sfx_None || id >= NUMSFX) return -1;

    int lump_num = I_GetSfxLumpNum(&S_sfx[id]);
    int lump_length = W_LumpLength(lump_num);

    uint8_t *data = W_CacheLumpNum(lump_num, PU_STATIC); // sfx data
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

    SetSoundVolume(channel->sound, (float)vol / 127.0f);
    PlaySound(channel->sound);

    return channel_number;
}

/// @brief Stops a SFX in a channel
/// @param handle Channel handle
void I_StopSound(int handle) {
    StopSound(channels[handle].sound);
    UnloadSound(channels[handle].sound);

    channels[handle].sound = (Sound){0};
    channels[handle].active = false;
}

/// @brief Checks if the sound is playing
/// @param handle Channel handle
/// @return is it playing
int I_SoundIsPlaying(int handle)
{
    boolean playing = IsSoundPlaying(channels[handle].sound);
    if(!playing) I_StopSound(handle);
    return (int)playing;
}

void I_InitSound(void) {
    InitAudioDevice();
    I_InitMusic();
}

void I_ShutdownSound(void) {
    CloseAudioDevice();
}


// UNUSED FUNCTIONS:
void I_UpdateSound(void) {}
void I_SubmitSound(void) {}
void I_UpdateSoundParams
( int		handle,
  int		vol,
  int		sep,
  int		pitch ) {}


// answer: i forgot to write PlaySound() when rewriting my code into multiple files