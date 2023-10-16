#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "audio.h"
#include "input.h"
#include "memory_arena.h"
#include "v2.h"
#include "thread_pool.h"
#include "engine.h"
#include "graphics.h"

/*
 * The game code. Finally some fresh code :)
 *
 * NOTE: this is not the game state.
 */

class Game {
public:
    Game();
    ~Game();

    void init();
    void deinit();
    void update_and_render(software_framebuffer* framebuffer, f32 dt);
private: 
    /* game resources here */
};

#endif
