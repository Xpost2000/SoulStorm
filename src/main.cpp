/*
 * A new engine. A new project.
 *
 * Holy I can be tired.
 *
 * NOTE: I'm aware that the STL containers aren't super optimal, but
 *       I'm a little tired of rolling out new stuff right now so...
 */
#include <SDL2/SDL.h>

#include "common.h"
#include "audio.h"

void initialize() {
    SDL_Init(SDL_INIT_EVERYTHING);
    Audio::initialize();
}

void deinitialize() {
    Audio::deinitialize();
    SDL_Quit();
}

int main(int argc, char** argv) {
    initialize();
    deinitialize();
    return 0; 
}
