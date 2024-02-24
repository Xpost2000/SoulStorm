#include "common.h"
#include "engine.h" // for memory arena access
#include "virtual_file_system.h"

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
    local string     loaded_sample_filestrings[MAX_LOADED_SAMPLES] = {};
    local string     loaded_stream_filestrings[MAX_LOADED_STREAMS] = {};
    local f32        current_sound_volume                          = 0.5;
    local f32        current_music_volume                          = 0.5;
    local bool       subsystem_enabled                             = true;

    void disable(void) {
        subsystem_enabled = false;
    }
    void enable(void) {
        subsystem_enabled = true;
    }

    void initialize(void) {
        _debugprintf("Audio hi");
        Mix_Init(MIX_INIT_OGG | MIX_INIT_MOD);
        Mix_OpenAudio(44100, AUDIO_S16, 2, 2048);
        Mix_AllocateChannels(32);
    }

    void deinitialize(void) {
        stop_music();
        stop_sounds();
        Mix_Quit();
    }

    f32 get_volume_sound(void) {
        return current_sound_volume;
    }

    f32 get_volume_music(void) {
        return current_music_volume;
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

                    if (loaded_streams[index] == NULL) {
#ifndef EXPERIMENTAL_VFS
                        struct file_buffer filebuffer = VFS_read_entire_file(memory_arena_allocator(&game_arena), string_from_cstring((char*)filepath));
                        SDL_RWops*         rw         = SDL_RWFromConstMem(filebuffer.buffer, filebuffer.length);
                        Mix_Music* new_stream = Mix_LoadMUS_RW(rw, 1);
#else
                        Mix_Music* new_stream = Mix_LoadMUS(filepath);
#endif
                        loaded_streams[index] = new_stream;
                    }

                    break;
                }
            }

            if (load_required) {
#ifndef EXPERIMENTAL_VFS
                struct file_buffer filebuffer = VFS_read_entire_file(memory_arena_allocator(&game_arena), string_from_cstring((char*)filepath));
                SDL_RWops*         rw         = SDL_RWFromConstMem(filebuffer.buffer, filebuffer.length);
                Mix_Music* new_stream = Mix_LoadMUS_RW(rw, 1);
#else
                Mix_Music* new_stream = Mix_LoadMUS(filepath);
#endif

                if (new_stream) {
                    _debugprintf("new stream: %p", new_stream);
                    loaded_stream_filestrings[loaded_stream_count] = memory_arena_push_string(&Global_Engine()->main_arena, string_from_cstring((char*)filepath));
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

                    if (loaded_samples[index] == NULL) {
#ifndef EXPERIMENTAL_VFS
                        Mix_Chunk* new_chunk = Mix_LoadWAV(filepath);
#else
                        struct file_buffer filebuffer = VFS_read_entire_file(heap_allocator(), string_from_cstring((char*)filepath));
                        SDL_RWops*         rw         = SDL_RWFromConstMem(filebuffer.buffer, filebuffer.length);
                        Mix_Chunk* new_chunk = Mix_LoadWAV_RW(rw, 1);
#endif
                        loaded_samples[index] = new_chunk;
                    }

                    break;
                }
            }

            if (load_required) {
#ifndef EXPERIMENTAL_VFS
                Mix_Chunk* new_chunk = Mix_LoadWAV(filepath);
#else
                struct file_buffer filebuffer = VFS_read_entire_file(heap_allocator(), string_from_cstring((char*)filepath));
                SDL_RWops*         rw         = SDL_RWFromConstMem(filebuffer.buffer, filebuffer.length);
                Mix_Chunk* new_chunk = Mix_LoadWAV_RW(rw, 1);
#endif

                if (new_chunk) {
                    _debugprintf("new sample: %p", new_chunk);
                    loaded_sample_filestrings[loaded_sample_count] = memory_arena_push_string(&Global_Engine()->main_arena, string_from_cstring((char*)filepath));
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

    void unload(Sound_ID sound) {
        s32 index = sound.index-1;

        if (sound.streaming) {
            Mix_FreeMusic(loaded_streams[index]);
            loaded_streams[index] = 0;
        } else {
            Mix_FreeChunk(loaded_samples[index]);
            loaded_samples[index] = 0;
        }
    }

    void stop_sounds(void) {
        Mix_HaltChannel(-1);
    }

    void stop_music(void) {
        Mix_HaltMusic();
    }

    void play(Sound_ID sound) {
        if (!subsystem_enabled)
            return;

        if (sound.index == 0) {
            _debugprintf("bad sound");
            return;
        }

        if (sound.streaming) {
            if (loaded_streams[sound.index-1] == NULL)
                loaded_streams[sound.index-1] = Mix_LoadMUS(loaded_stream_filestrings[sound.index-1].data);

            s32 status = Mix_PlayMusic(loaded_streams[sound.index-1], -1);
        } else {
            if (loaded_samples[sound.index-1] == NULL)
                loaded_samples[sound.index-1] = Mix_LoadWAV(loaded_sample_filestrings[sound.index-1].data);

            Mix_PlayChannel(ANY_CHANNEL, loaded_samples[sound.index-1], 0);
        }
    }

    bool music_playing(void) {
        return Mix_PlayingMusic();
    }

    void play_fadein(Sound_ID sound, s32 fadein_ms) {
        if (!subsystem_enabled)
            return;

        if (sound.streaming) {
            Mix_FadeInMusic(loaded_streams[sound.index-1], -1, fadein_ms);
        } else {
            Mix_FadeInChannel(ANY_CHANNEL, loaded_samples[sound.index-1], 0, fadein_ms);
        }
    }

    void stop_music_fadeout(s32 fadeout_ms) {
        Mix_FadeOutMusic(fadeout_ms);
    }

    void set_volume_sound(f32 v) {
        current_sound_volume = v;
        Mix_Volume(-1, (s32)(v * MIX_MAX_VOLUME));
    }

    void set_volume_music(f32 v) {
        current_music_volume = v;
        Mix_VolumeMusic((s32)(v * MIX_MAX_VOLUME));
    }

    local int _lua_bind_play_sound(lua_State* L) {
        s32 sound_index = luaL_checkinteger(L, 1);
        auto sound_id   = Sound_ID { false, sound_index };
        play(sound_id);
        return 0;
    }

    local int _lua_bind_play_music(lua_State* L) {
        s32 sound_index = luaL_checkinteger(L, 1);
        auto sound_id   = Sound_ID { true, sound_index };
        play(sound_id);
        return 0;
    }

    local int _lua_bind_play_music_fadein(lua_State* L) {
        s32 sound_index = luaL_checkinteger(L, 1);
        auto sound_id   = Sound_ID { true, sound_index };
        play_fadein(sound_id, luaL_checkinteger(L, 2));
        return 0;
    }

    local int _lua_bind_play_sound_fadein(lua_State* L) {
        s32 sound_index = luaL_checkinteger(L, 1);
        auto sound_id   = Sound_ID { false, sound_index };
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
        lua_register(L, "play_sound",          _lua_bind_play_sound);
        lua_register(L, "play_sound_fadein",   _lua_bind_play_sound_fadein);
        lua_register(L, "play_music",          _lua_bind_play_music);
        lua_register(L, "play_music_fadein",   _lua_bind_play_sound_fadein);
        lua_register(L, "stop_music_fadeout",  _lua_bind_stop_music_fadeout);
        lua_register(L, "stop_music",          _lua_bind_stop_music);
        lua_register(L, "music_playing",       _lua_bind_music_playing);
    }
}
