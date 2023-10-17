CC=g++
CFLAGS=-w -Wno-unused -Wno-unused-but-set-variable -std=c++11
CLIBS=-lmingw32 -L./dependencies/x86-64/lib/ -L./dependencies/x86-64/bin/ -I./dependencies/ -I./dependencies/x86-64/include/ -lOpenGL32 -lglew32 -lSDL2main -lSDL2 -lSDL2_mixer -msse4
ITCHPROJECT=xpost2000/untitled-project

HEADER_FILES= src/audio.h src/memory_arena.h src/v2.h src/input.h src/string.h src/thread_pool.h src/engine.h src/graphics.h src/allocators.h src/serializer.h src/entity.h
SOURCE_FILES= src/main.cpp src/audio.cpp src/memory_arena.cpp src/v2.cpp src/input.cpp src/string.cpp src/thread_pool.cpp src/engine.cpp src/graphics.cpp src/prng.cpp src/game.cpp src/allocators.cpp src/serializer.cpp src/entity.cpp

game-debug.exe: $(SOURCE_FILES) $(HEADER_FILES)
	$(CC) -o $@  $(SOURCE_FILES) $(CFLAGS) $(CLIBS) -m64 -ggdb3
game.exe: $(SOURCE_FILES) $(HEADER_FILES)
	$(CC) -o $@ $(SOURCE_FILES) $(CFLAGS) $(CLIBS)
run: game.exe
	./game.exe
run-debug: game-debug.exe
	./game-debug.exe
