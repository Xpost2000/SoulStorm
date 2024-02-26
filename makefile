# for gcc/clang based build systems.
# for emscripten... Great question! I hope there are no problems!
#
# If you're wanting to build on windows. Just use the msbuild scripts
# it will probably not break.
#
CC=g++
MAKE=make
CFLAGS=
CLIBS=
TARGET=
EXEC_EXT=
EXEC_PATH_PREPEND=
DISCORD_INTEGRATION=NO

ifeq ($(OS),Windows_NT)
	TARGET:=WIN32
	EXEC_EXT:=.exe
else
	TARGET:=LINUX_GENERIC
	EXEC_EXT:=.out
	EXEC_PATH_PREPEND=./
endif

.phony: all clean run run-debug docgen


# CC=clang++
CFLAGS:=-Wno-unused -Wno-unused-but-set-variable -std=c++17 -w

ifeq ($(TARGET), WIN32)
	CLIBS:=-lmingw32 -L./dependencies/x86-64/lib/ -L./dependencies/x86-64/bin/ -I./glad/include/\
		   -I./dependencies/ -I./dependencies/x86-64/include -I./dependencies/x86-64/include/SDL2\
		   -ld3d11 -ld3dcompiler -ldxguid -lOpenGL32 -lSDL2main -lSDL2 -lSDL2_mixer -llua54 -msse4 -m64
else
	CLIBS:=-I./glad/include/ -I./dependencies/ -I./dependencies/x86-64/include -lGL -lGLEW\
		   -lSDL2main -lSDL2 -lSDL2_mixer -llua5.4 -msse4 -m64
endif

ifeq ($(CC), emcc)
	# NOTE: a provided prebuilt version of lua is used.
	CLIBS:=-lSDL2_mixer -lSDL2 -s USE_SDL_MIXER=2 -llua -s USE_SDL=2 -s USE_WEBGL2=1
	CFLAGS:=-I./lua54src/include -I./dependencies/ -L./dependencies/wasm -s INITIAL_MEMORY=256MB\
			--preload-file res --preload-file stages --preload-file data.bigfile --preload-file controls.lua --preload-file preferences.lua -DRELEASE;
	EXEC_EXT:=.html
	DISCORD_INTEGRATION:=NO
endif


ifeq ($(DISCORD_INTEGRATION), YES)
	CLIBS+=-ldiscord_game_sdk
endif

ITCHPROJECT=xpost2000/soulstorm

all: game$(EXEC_EXT) game-debug$(EXEC_EXT)
clean:
	-rm ./build_intermediaries/*.o
	-rm game-debug$(EXEC_EXT)
	-rm game$(EXEC_EXT)
	-rm data.bigfile

HEADER_FILES= src/achievement_list.h \
		src/achievements.h \
		src/action_mapper.h \
		src/allocators.h \
		src/audio.h \
		src/camera.h \
		src/color.h \
		src/common.h \
		src/debug_ui.h \
		src/duffcoroutine.h \
		src/engine.h \
		src/entity.h \
		src/entity_prototypes.h \
		src/fade_transition.h \
		src/file_buffer.h \
		src/fixed_array.h \
		src/font_cache.h \
		src/game.h \
		src/game_preferences.h \
		src/game_state.h \
		src/game_task_scheduler.h \
		src/game_ui.h \
		src/graphics_assets.h \
		src/graphics_common.h \
		src/graphics_driver.h \
		src/graphics_driver_software.h \
		src/graphics_driver_opengl.h \
		src/graphics_driver_null.h \
		src/image_buffer.h \
		src/input.h \
		src/lightmask_buffer.h \
		src/main_menu_mode.h \
		src/memory_arena.h \
		src/prng.h \
		src/render_commands.h \
		src/save_data.h \
		src/sdl_scancode_table.h \
		src/serializer.h \
		src/shader_effect_types.h \
		src/software_renderer.h \
		src/particle_system.h \
		src/stage.h \
		src/virtual_file_system.h \
		src/discord_rich_presence_integration.h \
		src/stage_list.h \
		src/stages.h \
		src/string.h \
		src/thread_pool.h \
		src/title_screen_mode.h \
		Bigfilepacker/bigfile.h \
		src/v2.h

# src/audio.h src/memory_arena.h src/v2.h src/input.h src/string.h src/thread_pool.h src/engine.h src/graphics.h src/allocators.h src/serializer.h src/entity.h src/common.h
OBJECT_FILES=./build_intermediaries/achievements.o \
			 ./build_intermediaries/action_mapper.o \
			 ./build_intermediaries/allocators.o \
			 ./build_intermediaries/audio.o \
			 ./build_intermediaries/camera.o \
			 ./build_intermediaries/color.o \
			 ./build_intermediaries/common.o \
			 ./build_intermediaries/debug_ui.o \
			 ./build_intermediaries/engine.o \
			 ./build_intermediaries/entity.o \
			 ./build_intermediaries/entity_prototypes.o \
			 ./build_intermediaries/fade_transition.o \
			 ./build_intermediaries/game.o \
			 ./build_intermediaries/bigfile.o \
			 ./build_intermediaries/glad.o \
			 ./build_intermediaries/game_task_scheduler.o \
			 ./build_intermediaries/game_ui.o \
			 ./build_intermediaries/graphics_assets.o \
			 ./build_intermediaries/graphics_driver.o \
			 ./build_intermediaries/graphics_driver_null.o \
			 ./build_intermediaries/graphics_driver_opengl.o \
			 ./build_intermediaries/graphics_driver_software.o \
			 ./build_intermediaries/v2_lua_bindings.o \
			 ./build_intermediaries/entity_lua_bindings.o \
			 ./build_intermediaries/game_lua_bindings.o \
			 ./build_intermediaries/virtual_file_system.o \
			 ./build_intermediaries/particle_system.o \
			 ./build_intermediaries/particle_system_lua_bindings.o \
			 ./build_intermediaries/input.o \
			 ./build_intermediaries/lightmask_buffer.o \
			 ./build_intermediaries/main.o \
			 ./build_intermediaries/memory_arena.o \
			 ./build_intermediaries/prng.o \
			 ./build_intermediaries/render_commands.o \
			 ./build_intermediaries/serializer.o \
			 ./build_intermediaries/software_renderer.o \
			 ./build_intermediaries/discord_rich_presence_integration.o \
			 ./build_intermediaries/stage.o \
			 ./build_intermediaries/string.o \
			 ./build_intermediaries/thread_pool.o \
			 ./build_intermediaries/v2.o

DOBJECT_FILES=./build_intermediaries/achievements_debug.o \
			  ./build_intermediaries/action_mapper_debug.o \
			  ./build_intermediaries/allocators_debug.o \
			  ./build_intermediaries/audio_debug.o \
			  ./build_intermediaries/camera_debug.o \
			  ./build_intermediaries/color_debug.o \
			  ./build_intermediaries/common_debug.o \
			  ./build_intermediaries/debug_ui_debug.o \
			  ./build_intermediaries/engine_debug.o \
			  ./build_intermediaries/entity_debug.o \
			  ./build_intermediaries/entity_prototypes_debug.o \
			  ./build_intermediaries/fade_transition_debug.o \
			  ./build_intermediaries/game_debug.o \
			  ./build_intermediaries/bigfile_debug.o \
			  ./build_intermediaries/glad_debug.o \
			  ./build_intermediaries/game_task_scheduler_debug.o \
			  ./build_intermediaries/game_ui_debug.o \
			  ./build_intermediaries/graphics_assets_debug.o \
			  ./build_intermediaries/graphics_driver_debug.o \
			  ./build_intermediaries/graphics_driver_null_debug.o \
			  ./build_intermediaries/graphics_driver_opengl_debug.o \
			  ./build_intermediaries/graphics_driver_software_debug.o \
			  ./build_intermediaries/v2_lua_bindings_debug.o \
			  ./build_intermediaries/entity_lua_bindings_debug.o \
			  ./build_intermediaries/game_lua_bindings_debug.o \
			  ./build_intermediaries/virtual_file_system_debug.o \
			  ./build_intermediaries/particle_system_debug.o \
			  ./build_intermediaries/particle_system_lua_bindings_debug.o \
			  ./build_intermediaries/input_debug.o \
			  ./build_intermediaries/lightmask_buffer_debug.o \
			  ./build_intermediaries/main_debug.o \
			  ./build_intermediaries/memory_arena_debug.o \
			  ./build_intermediaries/prng_debug.o \
			  ./build_intermediaries/render_commands_debug.o \
			  ./build_intermediaries/serializer_debug.o \
			  ./build_intermediaries/software_renderer_debug.o \
			  ./build_intermediaries/discord_rich_presence_integration_debug.o \
			  ./build_intermediaries/stage_debug.o \
			  ./build_intermediaries/string_debug.o \
			  ./build_intermediaries/thread_pool_debug.o \
			  ./build_intermediaries/v2_debug.o

ifeq ($(TARGET), WIN32)
	HEADER_FILES  += src/graphics_driver_d3d11.h
	OBJECT_FILES  += ./build_intermediaries/graphics_driver_d3d11.o
	DOBJECT_FILES += ./build_intermediaries/graphics_driver_d3d11_debug.o
endif

./build_intermediaries/:
	-mkdir ./build_intermediaries/

./game-debug: CFLAGS += -ggdb3
./game:       CFLAGS += -O2 -DRELEASE

## modules 
./build_intermediaries/achievements.o: src/achievements.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/achievements.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/action_mapper.o: src/action_mapper.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/action_mapper.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/allocators.o: src/allocators.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/allocators.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/audio.o: src/audio.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/audio.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/camera.o: src/camera.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/camera.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/color.o: src/color.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/color.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/common.o: src/common.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/common.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/debug_ui.o: src/debug_ui.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/debug_ui.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/engine.o: src/engine.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/engine.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/entity.o: src/entity.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/entity.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/entity_prototypes.o: src/entity_prototypes.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/entity_prototypes.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/fade_transition.o: src/fade_transition.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/fade_transition.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/game.o: src/game.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/game.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/bigfile.o: Bigfilepacker/bigfile.cpp $(HEADER_FILES)
	$(CC) -o $@ -c Bigfilepacker/bigfile.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/glad.o: glad/src/glad.c $(HEADER_FILES)
	$(CC) -o $@ -c glad/src/glad.c $(CFLAGS) $(CLIBS) 
./build_intermediaries/game_task_scheduler.o: src/game_task_scheduler.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/game_task_scheduler.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/game_ui.o: src/game_ui.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/game_ui.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_assets.o: src/graphics_assets.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_assets.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_driver.o: src/graphics_driver.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_driver.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_driver_null.o: src/graphics_driver_null.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_driver_null.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_driver_opengl.o: src/graphics_driver_opengl.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_driver_opengl.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_driver_d3d11.o: src/graphics_driver_d3d11.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_driver_d3d11.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_driver_software.o: src/graphics_driver_software.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_driver_software.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/v2_lua_bindings.o: src/v2_lua_bindings.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/v2_lua_bindings.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/entity_lua_bindings.o: src/entity_lua_bindings.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/entity_lua_bindings.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/game_lua_bindings.o: src/game_lua_bindings.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/game_lua_bindings.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/virtual_file_system.o: src/virtual_file_system.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/virtual_file_system.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/particle_system.o: src/particle_system.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/particle_system.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/particle_system_lua_bindings.o: src/particle_system_lua_bindings.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/particle_system_lua_bindings.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/input.o: src/input.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/input.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/lightmask_buffer.o: src/lightmask_buffer.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/lightmask_buffer.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/main.o: src/main.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/main.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/memory_arena.o: src/memory_arena.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/memory_arena.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/prng.o: src/prng.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/prng.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/render_commands.o: src/render_commands.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/render_commands.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/serializer.o: src/serializer.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/serializer.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/software_renderer.o: src/software_renderer.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/software_renderer.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/discord_rich_presence_integration.o: src/discord_rich_presence_integration.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/discord_rich_presence_integration.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/stage.o: src/stage.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/stage.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/string.o: src/string.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/string.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/thread_pool.o: src/thread_pool.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/thread_pool.cpp $(CFLAGS) $(CLIBS) 
./build_intermediaries/v2.o: src/v2.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/v2.cpp $(CFLAGS) $(CLIBS) 

# NOTE: could be made as one target... with suffixing
./build_intermediaries/achievements_debug.o: src/achievements.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/achievements.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/action_mapper_debug.o: src/action_mapper.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/action_mapper.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/allocators_debug.o: src/allocators.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/allocators.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/audio_debug.o: src/audio.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/audio.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/camera_debug.o: src/camera.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/camera.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/color_debug.o: src/color.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/color.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/common_debug.o: src/common.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/common.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/debug_ui_debug.o: src/debug_ui.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/debug_ui.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/engine_debug.o: src/engine.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/engine.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/entity_debug.o: src/entity.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/entity.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/entity_prototypes_debug.o: src/entity_prototypes.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/entity_prototypes.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/fade_transition_debug.o: src/fade_transition.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/fade_transition.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/game_debug.o: src/game.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/game.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/bigfile_debug.o: Bigfilepacker/bigfile.cpp $(HEADER_FILES)
	$(CC) -o $@ -c Bigfilepacker/bigfile.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/glad_debug.o: glad/src/glad.c $(HEADER_FILES)
	$(CC) -o $@ -c glad/src/glad.c -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/game_task_scheduler_debug.o: src/game_task_scheduler.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/game_task_scheduler.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/game_ui_debug.o: src/game_ui.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/game_ui.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_assets_debug.o: src/graphics_assets.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_assets.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_driver_debug.o: src/graphics_driver.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_driver.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_driver_null_debug.o: src/graphics_driver_null.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_driver_null.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_driver_opengl_debug.o: src/graphics_driver_opengl.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_driver_opengl.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_driver_d3d11_debug.o: src/graphics_driver_d3d11.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_driver_d3d11.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/graphics_driver_software_debug.o: src/graphics_driver_software.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/graphics_driver_software.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/v2_lua_bindings_debug.o: src/v2_lua_bindings.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/v2_lua_bindings.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/entity_lua_bindings_debug.o: src/entity_lua_bindings.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/entity_lua_bindings.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/game_lua_bindings_debug.o: src/game_lua_bindings.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/game_lua_bindings.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/virtual_file_system_debug.o: src/virtual_file_system.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/virtual_file_system.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/particle_system_debug.o: src/particle_system.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/particle_system.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/particle_system_lua_bindings_debug.o: src/particle_system_lua_bindings.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/particle_system_lua_bindings.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/input_debug.o: src/input.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/input.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/lightmask_buffer_debug.o: src/lightmask_buffer.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/lightmask_buffer.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/main_debug.o: src/main.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/main.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/memory_arena_debug.o: src/memory_arena.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/memory_arena.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/prng_debug.o: src/prng.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/prng.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/render_commands_debug.o: src/render_commands.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/render_commands.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/serializer_debug.o: src/serializer.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/serializer.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/software_renderer_debug.o: src/software_renderer.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/software_renderer.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/discord_rich_presence_integration_debug.o: src/discord_rich_presence_integration.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/discord_rich_presence_integration.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/stage_debug.o: src/stage.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/stage.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/string_debug.o: src/string.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/string.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/thread_pool_debug.o: src/thread_pool.cpp $(HEADER_FILES)
	$(CC) -o $@ -c src/thread_pool.cpp -g $(CFLAGS) $(CLIBS) 
./build_intermediaries/v2_debug.o: src/v2.cpp$(HEADER_FILES)
	$(CC) -o $@ -c src/v2.cpp -g $(CFLAGS) $(CLIBS) 

## end modules
docgen: src/lua_metagen.cpp
	$(CC) src/lua_metagen.cpp -o ./metagen
	./metagen
	pandoc -c ./retro.css -s lua_engine_api.md -o lua_engine_api.html

./Bigfilepacker$(EXEC_EXT): Bigfilepacker/bigfile.h Bigfilepacker/bigfile.cpp Bigfilepacker/main.cpp src/memory_arena.cpp src/string.cpp src/common.cpp src/allocators.cpp src/v2.cpp src/serializer.cpp
	$(CC) $(CFLAGS) -o $@ Bigfilepacker/bigfile.cpp Bigfilepacker/main.cpp src/memory_arena.cpp\
		src/string.cpp src/common.cpp src/allocators.cpp src/v2.cpp src/serializer.cpp

./data.bigfile: ./Bigfilepacker$(EXEC_EXT)
	@echo Building bigfile packer.
	$(EXEC_PATH_PREPEND)Bigfilepacker$(EXEC_EXT) data.bigfile res/ stages/

./game$(EXEC_EXT): ./data.bigfile docgen $(OBJECT_FILES) $(HEADER_FILES)
	@echo Building release build.
	$(CC) -o $@ $(OBJECT_FILES) $(CFLAGS) $(CLIBS) 

./game-debug$(EXEC_EXT): docgen $(DOBJECT_FILES) $(HEADER_FILES)
	@echo Building debug build.
	$(CC) -o $@  $(DOBJECT_FILES) $(CFLAGS) $(CLIBS) 

run: ./game$(EXEC_EXT)
	$(EXEC_PATH_PREPEND)game
run-debug: ./game-debug$(EXEC_EXT)
	$(EXEC_PATH_PREPEND)game-debug
