CC=g++
# CC=clang++
CFLAGS=-w -Wno-unused -Wno-unused-but-set-variable -std=c++11
CLIBS=-lmingw32 -L./dependencies/x86-64/lib/ -L./dependencies/x86-64/bin/ -I./dependencies/ -I./dependencies/x86-64/include -I./dependencies/x86-64/include/SDL2 -lOpenGL32 -lglew32 -lSDL2main -lSDL2 -lSDL2_mixer -llua54 -msse4
ITCHPROJECT=xpost2000/untitled-project

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
		src/stage.h \
		src/stage_list.h \
		src/stages.h \
		src/string.h \
		src/thread_pool.h \
		src/title_screen_mode.h \
		src/v2.h

# src/audio.h src/memory_arena.h src/v2.h src/input.h src/string.h src/thread_pool.h src/engine.h src/graphics.h src/allocators.h src/serializer.h src/entity.h src/common.h
SOURCE_FILES= src/achievements.cpp \
		src/action_mapper.cpp \
		src/allocators.cpp \
		src/audio.cpp \
		src/camera.cpp \
		src/color.cpp \
		src/common.cpp \
		src/debug_ui.cpp \
		src/engine.cpp \
		src/entity.cpp \
		src/entity_prototypes.cpp \
		src/fade_transition.cpp \
		src/game.cpp \
		src/game_task_scheduler.cpp \
		src/game_ui.cpp \
		src/graphics_assets.cpp \
		src/graphics_driver.cpp \
		src/graphics_driver_software.cpp \
		src/input.cpp \
		src/lightmask_buffer.cpp \
		src/main.cpp \
		src/memory_arena.cpp \
		src/prng.cpp \
		src/render_commands.cpp \
		src/serializer.cpp \
		src/software_renderer.cpp \
		src/stage.cpp \
		src/string.cpp \
		src/thread_pool.cpp \
		src/v2.cpp

# src/main.cpp src/audio.cpp src/memory_arena.cpp src/v2.cpp src/input.cpp src/string.cpp src/thread_pool.cpp src/engine.cpp src/graphics.cpp src/prng.cpp src/game.cpp src/allocators.cpp src/serializer.cpp src/entity.cpp

game-debug.exe: $(SOURCE_FILES) $(HEADER_FILES)
	$(CC) -o $@  $(SOURCE_FILES) $(CFLAGS) $(CLIBS) -m64 -ggdb3
game.exe: $(SOURCE_FILES) $(HEADER_FILES)
	$(CC) -o $@ $(SOURCE_FILES) $(CFLAGS) $(CLIBS) -O2
run: game.exe
	./game.exe
run-debug: game-debug.exe
	./game-debug.exe
