#include "common.h"
#include "audio.h"

#include <SDL2/SDL_mixer.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace Audio {
    local Mix_Chunk* loaded_samples[MAX_LOADED_SAMPLES]       = {};
    local u32        loaded_sample_hashes[MAX_LOADED_SAMPLES] = {};
    local Mix_Music* loaded_streams[MAX_LOADED_STREAMS]       = {};
    local u32        loaded_stream_hashes[MAX_LOADED_SAMPLES] = {};
    local s32        loaded_sample_count                      = 0;
    local s32        loaded_stream_count                      = 0;

    void initialize(void) {
        _debugprintf("Audio hi");
        Mix_Init(MIX_INIT_OGG | MIX_INIT_MOD);
        Mix_OpenAudio(44100, AUDIO_S16, 2, 2048);
    }

    void deinitialize(void) {
        Mix_Quit();
    }

    Sound_ID load(const char* filepath, const bool streamed) {
        Sound_ID result    = {};
        u32      path_hash = hash_bytes_fnv1a((u8*)filepath, strlen(filepath));

        if (streamed) {
            result.streaming = 1;

            bool load_required = true;
            for (s32 index = 0; index < loaded_stream_count; ++index) {
                if (path_hash == loaded_stream_hashes[index]) {
                    result.index  = index+1;
                    load_required = false;
                    break;
                }
            }

            if (load_required) {
                // struct file_buffer filebuffer = read_entire_file(memory_arena_allocator(&game_arena), filepath);
                // SDL_RWops*         rw         = SDL_RWFromConstMem(filebuffer.buffer, filebuffer.length);
                Mix_Music* new_stream = Mix_LoadMUS(filepath);
                // Mix_Music* new_stream = Mix_LoadMUS_RW(rw, 1);

                if (new_stream) {
                    _debugprintf("new stream: %p", new_stream);
                    loaded_streams[loaded_stream_count]       = new_stream;
                    loaded_stream_hashes[loaded_stream_count] = path_hash;

                    loaded_stream_count++;
                    result.index = loaded_stream_count;
                }
            }
        } else {
            bool load_required = true;
            for (s32 index = 0; index < loaded_sample_count; ++index) {
                if (path_hash == loaded_sample_hashes[index]) {
                    result.index  = index+1;
                    load_required = false;
                    break;
                }
            }

            if (load_required) {
                // struct file_buffer filebuffer = read_entire_file(memory_arena_allocator(&scratch_arena), filepath);
                // SDL_RWops*         rw         = SDL_RWFromConstMem(filebuffer.buffer, filebuffer.length);
                Mix_Chunk* new_chunk = Mix_LoadWAV(filepath);
                // Mix_Chunk* new_chunk = Mix_LoadWAV_RW(rw, 1);
                if (new_chunk) {
                    _debugprintf("new sample: %p", new_chunk);
                    loaded_samples[loaded_sample_count]       = new_chunk;
                    loaded_sample_hashes[loaded_sample_count] = path_hash;

                    loaded_sample_count++;
                    result.index = loaded_sample_count;
                } else {
                    _debugprintf("bad load: %s\n", filepath);
                }
            }
        }

        return result;
    }

    void stop_music(void) {
        Mix_HaltMusic();
    }

    void play(Sound_ID sound) {
        _debugprintf("play sound hit?");
        if (sound.index == 0) {
            _debugprintf("bad sound");
            return;
        }

        if (sound.streaming) {
            s32 status = Mix_PlayMusic(loaded_streams[sound.index-1], -1);
            _debugprintf("HI music?: %p (%s)", loaded_streams[sound.index-1], Mix_GetError());
        } else {
            Mix_PlayChannel(ANY_CHANNEL, loaded_samples[sound.index-1], 0);
            _debugprintf("HI: %p (%d)", loaded_samples[sound.index-1], sound.index);
        }
    }

    bool music_playing(void) {
        return Mix_PlayingMusic();
    }

    void play_fadein(Sound_ID sound, s32 fadein_ms) {
        if (sound.streaming) {
            Mix_FadeInMusic(loaded_streams[sound.index-1], -1, fadein_ms);
            _debugprintf("fadein HI music?: %p (%s)", loaded_streams[sound.index-1], Mix_GetError());
        } else {
            Mix_FadeInChannel(ANY_CHANNEL, loaded_samples[sound.index-1], 0, fadein_ms);
        }
    }

    void stop_music_fadeout(s32 fadeout_ms) {
        Mix_FadeOutMusic(fadeout_ms);
    }

    void set_volume_sound(f32 v) {
        Mix_Volume(-1, (s32)(v * MIX_MAX_VOLUME));
    }

    void set_volume_music(f32 v) {
        Mix_VolumeMusic((s32)(v * MIX_MAX_VOLUME));
    }

    local int _lua_bind_load_sound(lua_State* L) {
const         char* filepath = lua_tostring(L, 1);
        auto sound_id = load(filepath, false);
        lua_pushinteger(L, sound_id.index);
        return 1;
    }

    local int _lua_bind_load_music(lua_State* L) {
        const char* filepath = lua_tostring(L, 1);
        auto sound_id = load(filepath, true);
        lua_pushinteger(L, sound_id.index);
        return 1;
    }

    local int _lua_bind_play_sound(lua_State* L) {
        s32 sound_index = luaL_checkinteger(L, 1);
        auto sound_id   = Sound_ID { false, sound_index+1 };
        play(sound_id);
        return 0;
    }

    local int _lua_bind_play_music(lua_State* L) {
        s32 sound_index = luaL_checkinteger(L, 1);
        auto sound_id   = Sound_ID { true, sound_index+1 };
        play(sound_id);
        return 0;
    }

    local int _lua_bind_play_music_fadein(lua_State* L) {
        s32 sound_index = luaL_checkinteger(L, 1);
        auto sound_id   = Sound_ID { true, sound_index+1 };
        play_fadein(sound_id, luaL_checkinteger(L, 2));
        return 0;
    }

    local int _lua_bind_play_sound_fadein(lua_State* L) {
        s32 sound_index = luaL_checkinteger(L, 1);
        auto sound_id   = Sound_ID { false, sound_index+1 };
        play_fadein(sound_id, luaL_checkinteger(L, 2));
        return 0;
    }

    local int _lua_bind_stop_music_fadeout(lua_State* L) {
        stop_music_fadeout(luaL_checkinteger(L, 1));
        return 0;
    }

    local int _lua_bind_stop_music(lua_State* L) {
        stop_music_fadeout(luaL_checkinteger(L, 1));
        return 0;
    }

    local int _lua_bind_music_playing(lua_State* L) {
        lua_pushboolean(L, music_playing());
        return 1;
    }

    void bind_audio_lualib(lua_State* L) {
        lua_register(L, "load_sound",          _lua_bind_load_sound);
        lua_register(L, "load_music",          _lua_bind_load_music);
        lua_register(L, "play_sound",          _lua_bind_play_sound);
        lua_register(L, "play_sound_fadein",   _lua_bind_play_sound_fadein);
        lua_register(L, "play_music",          _lua_bind_play_music);
        lua_register(L, "play_music_fadein",   _lua_bind_play_sound_fadein);
        lua_register(L, "stop_music_fadeout",  _lua_bind_stop_music_fadeout);
        lua_register(L, "stop_music",          _lua_bind_stop_music);
        lua_register(L, "music_playing",       _lua_bind_music_playing);
    }
}
