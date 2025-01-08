#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"
#define MAX_LOADED_SAMPLES (2048)
#define MAX_LOADED_STREAMS (512)
#define ANY_CHANNEL (-1)

// NOTE(jerry):
// too heavily tied to SDL_Mixer, which isn't the best audio API
// but I admittedly don't know enough about audio programming to
// know what a good design is, but this is surely a simple design.
struct lua_State;
namespace Audio {
    struct Sound_ID {
        u8  streaming;
        s32 index;
    };
    bool sound_id_match(Sound_ID a, Sound_ID b);

    void initialize();
    void deinitialize();

    Sound_ID load(const char* path, const bool streamed=false);
    void     unload(Sound_ID sound);

    void play(const Sound_ID sound);
    void pause_music(void);
    void resume_music(void);

    void stop_sounds();
    void stop_music();
    void stop_music_fadeout(const s32 fadeout_ms);
    void play_fadein(const Sound_ID sound, const s32 fadein_ms);

    // NOTE(jerry): helper to cover common case...
    void play_music_transition_into(const Sound_ID sound, const s32 fadeout_ms, const s32 fadein_ms);

    void disable(void);
    void enable(void);

    f32  get_volume_sound(void);
    f32  get_volume_music(void);
    void set_volume_sound(f32 v);
    void set_volume_music(f32 v);

    bool music_playing(void);
    Sound_ID current_music_sound(void);

    void bind_audio_lualib(lua_State* L);
}

#endif
