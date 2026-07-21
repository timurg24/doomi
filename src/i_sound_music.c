#include "i_sound.h"
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
#include <adlmidi.h>

#define MAX_REGISTERED_SONGS 1
#define MUSIC_FRAMES 2048
#define MUSIC_CHANNELS 2

typedef struct {
    void *data;
    int size;
    boolean is_playing;
    boolean loop;
} registered_song;

int music_volume = 0;

static registered_song songs[MAX_REGISTERED_SONGS];
static AudioStream music_stream;
static short music_buffer[MUSIC_FRAMES * MUSIC_CHANNELS];

static struct ADL_MIDIPlayer *music_player = NULL;

static int active_handle = -1;


/// @brief Initializes the music
void I_InitMusic(void) {
    if(music_player != NULL) return;
    music_player = adl_init(44100);
    if(!music_player) I_Error("Error while initialization (libADLMIDI): %s", adl_errorString());
    adl_switchEmulator(music_player, ADLMIDI_EMU_NUKED);

    SetAudioStreamBufferSizeDefault(MUSIC_FRAMES);

    // create music stream
    music_stream = LoadAudioStream(
        44100,
        16,
        2
    );

    if(!IsAudioStreamValid(music_stream)) I_Error("Could not create music AudioStream");
}

/// @brief Shutdowns the music 
void I_ShutdownMusic(void) {
    if (IsAudioStreamValid(music_stream))
    {
        StopAudioStream(music_stream);
        UnloadAudioStream(music_stream);
        music_stream = (AudioStream){0};
    }

    if (music_player != NULL)
    {
        adl_close(music_player);
        music_player = NULL;
    }
}

static boolean I_ValidSongHandle(int handle)
{
    return handle >= 0
        && handle < MAX_REGISTERED_SONGS
        && songs[handle].data != NULL;
}

/// @brief Returns a handle for a new song
/// @return Blank handle
int I_FindOpenSong(void) {
    for(int i = 0; i < MAX_REGISTERED_SONGS; i++) {
        if(songs[i].data == NULL) return i;
    }

    return -1;
}

/// @brief Registers a song
/// @param data Song MUS
/// @param size Song size
/// @return Handle
int I_RegisterSong(void *data, int size) {
    if(data == NULL || size <= 0) return -1;
    int handle = I_FindOpenSong();
    if(I_ValidSongHandle(handle)) fprintf(stderr, "I_RegisterSong: no free song slots available\n");

    songs[handle].data = data;
    songs[handle].size = size;
    songs[handle].is_playing = false;
    songs[handle].loop = false;
    return handle;
}

/// @brief Unregisters a song
/// @param handle 
void I_UnRegisterSong(int handle) {
    if (!I_ValidSongHandle(handle))
        return;

    if (handle == active_handle)
        I_StopSong(handle);

    songs[handle].data = NULL;
    songs[handle].is_playing = false;
    songs[handle].loop = false;
    songs[handle].size = 0;
}

/// @brief Loads new PCM data into the audio stream
void I_UpdateMusic(void) {
    if(!songs[active_handle].is_playing) return;
    if(!IsAudioStreamProcessed(music_stream)) return;

    while(IsAudioStreamProcessed(music_stream)) {
        int samples = adl_play(
            music_player,
            MUSIC_FRAMES * MUSIC_CHANNELS,
            music_buffer
        );

        if(samples <= 0) { // eend of music
            if(songs[active_handle].loop) {
                adl_positionRewind(music_player);
                continue;
            }

            songs[active_handle].is_playing = false;
            StopAudioStream(music_stream);
            return;
        }

        UpdateAudioStream(
            music_stream,
            music_buffer,
            samples / 2
        );
    }
}

/// @brief Plays a new song
/// @param handle Song handle
/// @param looping Looping
void I_PlaySong(int handle, int looping) {
    printf("I_PlaySong(handle=%d)\n", handle);
    if(!I_ValidSongHandle(handle) || songs[handle].data == NULL) return;
    if(music_player == NULL) I_Error("Attempted to play music before initialization");
    int result = adl_openData(
        music_player,
        songs[handle].data,
        songs[handle].size
    );

    if(result < 0) I_Error("Error when attemtping to play music (libADLMIDID): %s", adl_errorInfo(music_player));

    songs[handle].loop = looping != 0;
    songs[handle].is_playing = true;

    adl_setLoopEnabled(music_player, songs[handle].loop);
    adl_setLoopCount(music_player, songs[handle].loop ? -1 : 0);

    active_handle = handle;

    unsigned char *p = songs[handle].data;

    printf("%02X %02X %02X %02X %02X %02X %02X %02X\n",
        p[0], p[1], p[2], p[3],
        p[4], p[5], p[6], p[7]);
    printf("Song size: %d\n", songs[handle].size);
    PlayAudioStream(music_stream);
}

/// @brief Stops a song (!!!DO NOT DO THIS IN A LIVE PERFORMANCE!!!)
/// @param handle Song handle
void I_StopSong(int handle) {
    if(!I_ValidSongHandle(handle)) return;
    if(handle != active_handle) return;

    songs[handle].is_playing = false;
    StopAudioStream(music_stream);
    adl_positionRewind(music_player);
    active_handle = -1;
}

/// @brief Pauses a song...
/// @param handle Song handle
void I_PauseSong(int handle)
{
    if (!I_ValidSongHandle(handle))
        return;

    if (handle != active_handle || !songs[handle].is_playing)
        return;

    PauseAudioStream(music_stream);
}

/// @brief Resumes a song
/// @param handle Song handle
void I_ResumeSong(int handle)
{
    if (!I_ValidSongHandle(handle))
        return;

    if (handle != active_handle || !songs[handle].is_playing)
        return;
    ResumeAudioStream(music_stream);
}

void I_SetMusicVolume(int volume)
{
    if (volume < 0)
        volume = 0;
    else if (volume > 15)
        volume = 15;

    music_volume = volume;

    if (IsAudioStreamValid(music_stream))
    {
        SetAudioStreamVolume(
            music_stream,
            (float)volume / 15.0f
        );
    }
}
// ==UNUSED==
int I_QrySongPlaying(int handle)
{
    if (!I_ValidSongHandle(handle))
        return 0;
    return songs[handle].is_playing;
}