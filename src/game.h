#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "fixed_array.h"
#include "audio.h"
#include "input.h"
#include "memory_arena.h"
#include "v2.h"
#include "thread_pool.h"
#include "engine.h"
#include "graphics_driver.h"

#include "serializer.h"

#include "game_preferences.h"

// forward decl some opaque types
struct Game_State;
struct Game_Resources;

// since this is strictly animation only + the fact it
// does not affect any actual game state means I can just put this here.
enum Achievement_Notification_Phase {
    ACHIEVEMENT_NOTIFICATION_PHASE_APPEAR,
    ACHIEVEMENT_NOTIFICATION_PHASE_LINGER,
    ACHIEVEMENT_NOTIFICATION_PHASE_BYE,
};
struct Achievement_Notification {
    s32 phase;
    s32 id;
    f32 timer; // NOTE: Timer is in entity.h, but this is a simple thing so I'm not going to use it.
};
struct Achievement_Notification_State {
    Fixed_Array<Achievement_Notification> notifications;   
};

#include "save_data.h"

// TODO:
//
// I'd like to have a way to make more particle systems.
// that are preferably not hardcoded, or at least more flexible.
//
// I may just have a simple text file format through fscanf since it's
// not a big "deal" to user handle things.
//
// or have a very flat tokenizer to handle really really simple formats that
// I would keep in text or something like that.
//
// Since there's a lot of hardcoded data, and while some of it is justified
// something like the particles should obviously not require a re-compile.
//
// Might work on getting more stuff to be data oriented to be less painful.
// 
struct Entity_Loop_Update_Packet {
    Game_State* game_state;
    f32         dt;
};

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

    /*
      NOTE: this game / engine doesn't have a fully fledged UI system.

      It's just an IMGUI that's basically glued together, with these states
      to separate screens.

      This happens to be just fine when working with a controller though because
      you can't use a mouse to go through lots of stuff...
    */
    void update_and_render_pause_menu(struct render_commands* commands, f32 dt);
    void update_and_render_options_menu(struct render_commands* commands, f32 dt);
    void update_and_render_stage_select_menu(struct render_commands* commands, f32 dt);
    void update_and_render_achievements_menu(struct render_commands* commands, f32 dt);
    void update_and_render_game_death_maybe_retry_menu(struct render_commands* commands, f32 dt);
    void update_and_render_achievement_notifications(struct render_commands* commands, f32 dt);
    void update_and_render_confirm_back_to_main_menu(struct render_commands* commands, f32 dt);
    void update_and_render_confirm_exit_to_windows(struct render_commands* commands, f32 dt);

    // Game Modes
    void update_and_render_game_opening(struct render_commands* game_render_commands, struct render_commands* ui_render_commands, f32 dt);
    void update_and_render_game_main_menu(struct render_commands* game_render_commands, struct render_commands* ui_render_commands, f32 dt);
    void update_and_render_game_ingame(struct render_commands* game_render_commands, struct render_commands* ui_render_commands, f32 dt);
    void update_and_render_game_credits(struct render_commands* game_render_commands, struct render_commands* ui_render_commands, f32 dt);
    void update_and_render_game_title_screen(struct render_commands* game_render_commands, struct render_commands* ui_render_commands, f32 dt);

    void ingame_update_introduction_sequence(struct render_commands* commands, Game_Resources* resources, f32 dt);
    void ingame_update_complete_stage_sequence(struct render_commands* commands, Game_Resources* resources, f32 dt);

    // NOTE: most of these should be easy to parallesize.
    void handle_all_bullet_collisions(f32 dt);
    void handle_player_enemy_collisions(f32 dt);
    void handle_player_pickup_collisions(f32 dt);
    void handle_all_explosions(f32 dt);
    void handle_all_lasers(f32 dt);
    void handle_all_dead_entities(f32 dt);
    void handle_bomb_usage(f32 dt);

    /*
     * NOTE: primarily with the player as context
     *
     * kill_all_* is scoreless. Meant to just easily cleanup a wave.
     */
    void kill_all_bullets();
    void kill_all_enemies();
    void convert_bullets_to_score_pickups(float radius=99999);
    void convert_enemies_to_score_pickups(float radius=99999);

    void on_player_death();
    bool safely_resurrect_player();
    bool can_resurrect();

    void switch_screen(s32 screen);
    void switch_ui(s32 ui);
    bool can_access_stage(s32 id);

    void setup_stage_start();
    void reset_stage_simulation_state();
    void cleanup_game_simulation();
    void simulate_game_frame(Entity_Loop_Update_Packet* update_params);

    // The game will only utilize one save file
    // and auto save so this makes so much way easier.
    // the save file format is not very complicated honestly, since it just
    // needs to basically record what levels you beat, and what score you got.
    bool save_game();
    bool load_game();
    Save_File construct_save_data();
    Save_File serialize_game_state(struct binary_serializer* serializer);
    void update_from_save_data(Save_File* save_data);

    void notify_new_achievement(s32 id);

    Memory_Arena*   arena;
    Game_State*     state;
    Game_Resources* resources;
    bool initialized = false;

    // used for writing into from the settings before I
    // actually apply them.
    Game_Preferences temp_preferences;
    Achievement_Notification_State achievement_state;

    f32 total_playtime;
};

#endif
