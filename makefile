CC=g++
CFLAGS=-Werror -Wno-unused -Wno-unused-but-set-variable -Wall -std=c++11
CLIBS=-lmingw32 -L./dependencies/x86-64/lib/ -L./dependencies/x86-64/bin/ -I./dependencies/ -I./dependencies/x86-64/include/ -lOpenGL32 -lglew32 -lSDL2main -lSDL2 -lSDL2_mixer -msse4
ITCHPROJECT=xpost2000/untitled-project

SOURCE_FILES= src/main.cpp src/audio.cpp src/memory_arena.cpp src/v2.cpp src/input.cpp src/string.cpp src/thread_pool.cpp

game-debug.exe: $(SOURCE_FILES)
	$(CC) -o $@  $(SOURCE_FILES) $(CFLAGS) $(CLIBS) -m64 -ggdb3
game.exe: $(SOURCE_FILES)
	$(CC) -o $@ $(SOURCE_FILES) $(CFLAGS) $(CLIBS)
run: game.exe
	./game.exe
run-debug: game-debug.exe
	./game-debug.exe
