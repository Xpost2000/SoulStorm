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

// forward decl some opaque types
struct Game_State;
struct Game_Resources;

class Game {
public:
    Game();
    ~Game();

    void init();
    void deinit();

    // NOTE: I should probably avoid having an explicit reference to
    //       a software framebuffer. Honestly there's little to no reason why this should
    //       be here...
    void update_and_render(software_framebuffer* framebuffer, f32 dt);
private: 
    void update_and_render_pause_menu(struct render_commands* commands, f32 dt);
    void handle_all_explosions(f32 dt);
    void handle_all_lasers(f32 dt);
    void handle_all_dead_entities(f32 dt);

    Memory_Arena*   arena;
    Game_State*     state;
    Game_Resources* resources;
    /* game resources here */
};

#endif
