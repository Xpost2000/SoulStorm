@echo off

@rem Grr need better emscripten setup
@rem anyways here's the one liner to compile the whole game
@rem cause I use unity builds

@rem edit this line for emscripten path
echo This should be done on Windows cmd.exe
echo F:/emsdk/emsdk activate latest use this first

emcc
src/achievements.cpp
src/action_mapper.cpp
src/allocators.cpp
src/audio.cpp
src/camera.cpp
src/color.cpp
src/common.cpp
src/debug_ui.cpp
src/engine.cpp
src/entity.cpp
src/entity_prototypes.cpp
src/fade_transition.cpp
src/game.cpp
Bigfilepacker/bigfile.cpp
glad/src/glad.c
src/game_task_scheduler.cpp
src/game_ui.cpp
src/graphics_assets.cpp
src/graphics_driver.cpp
src/graphics_driver_null.cpp
src/graphics_driver_opengl.cpp
src/graphics_driver_software.cpp
src/v2_lua_bindings.cpp
src/entity_lua_bindings.cpp
src/game_lua_bindings.cpp
src/virtual_file_system.cpp
src/particle_system.cpp
src/particle_system_lua_bindings.cpp
src/input.cpp
src/lightmask_buffer.cpp
src/main.cpp
src/memory_arena.cpp
src/prng.cpp
src/render_commands.cpp
src/serializer.cpp
src/software_renderer.cpp
src/discord_rich_presence_integration.cpp
src/stage.cpp
src/string.cpp
src/thread_pool.cpp src/v2.cpp
-O2 -lSDL2_mixer -lSDL2 -s USE_SDL_MIXER=2 -s USE_SDL=2 -s USE_WEBGL2=1 -I./dependencies/ -o game.html -s INITIAL_MEMORY=256MB --preload-file res --preload-file areas --preload-file shops --preload-file dlg --preload-file scenes -DRELEASE;
