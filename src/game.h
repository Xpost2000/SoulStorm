#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "audio.h"
#include "input.h"
#include "memory_arena.h"
#include "v2.h"
#include "thread_pool.h"
#include "engine.h"
#include "graphics_driver.h"

#include "game_preferences.h"

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

    void init(Graphics_Driver* driver);
    void init_graphics_resources(Graphics_Driver* driver);
    void init_audio_resources();
    void deinit();

    // NOTE: I should probably avoid having an explicit reference to
    //       a software framebuffer. Honestly there's little to no reason why this should
    //       be here...
    void update_and_render(Graphics_Driver* driver, f32 dt);


    Game_Preferences preferences;
private: 
    void handle_ui_update_and_render(struct render_commands* commands, f32 dt);

    void update_and_render_pause_menu(struct render_commands* commands, f32 dt);
    void update_and_render_options_menu(struct render_commands* commands, f32 dt);
    void update_and_render_stage_select_menu(struct render_commands* commands, f32 dt);

    void update_and_render_game_opening(Graphics_Driver* driver, f32 dt);
    void update_and_render_game_main_menu(Graphics_Driver* driver, f32 dt);
    void update_and_render_game_ingame(Graphics_Driver* driver, f32 dt);
    void update_and_render_game_credits(Graphics_Driver* driver, f32 dt);
    void update_and_render_game_title_screen(Graphics_Driver* driver, f32 dt);

    void handle_all_explosions(f32 dt);
    void handle_all_lasers(f32 dt);
    void handle_all_dead_entities(f32 dt);

    bool can_access_stage(s32 id);

    Memory_Arena*   arena;
    Game_State*     state;
    Game_Resources* resources;
    bool initialized = false;

    // used for writing into from the settings before I
    // actually apply them.
    Game_Preferences temp_preferences;
};

#endif
