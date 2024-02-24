#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"
#define MAX_LOADED_SAMPLES (2048)
#define MAX_LOADED_STREAMS (512)
#define ANY_CHANNEL (-1)

// TODO: does not have a way to change volume!
struct lua_State;
namespace Audio {
    struct Sound_ID {
        u8  streaming;
        s32 index;
    };

    void initialize();
    void deinitialize();

    Sound_ID load(const char* path, const bool streamed=false);
    void     unload(Sound_ID sound);

    void play(const Sound_ID sound);

    void stop_sounds();
    void stop_music();
    void stop_music_fadeout(const s32 fadeout_ms);
    void play_fadein(const Sound_ID sound, const s32 fadein_ms);

    void disable(void);
    void enable(void);

    f32  get_volume_sound(void);
    f32  get_volume_music(void);
    void set_volume_sound(f32 v);
    void set_volume_music(f32 v);

    bool music_playing();

    void bind_audio_lualib(lua_State* L);
}

#endif
