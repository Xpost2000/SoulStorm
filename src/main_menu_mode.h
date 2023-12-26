#ifndef MAIN_MENU_MODE_H
#define MAIN_MENU_MODE_H

#include "particle_system.h"
#include "v2.h"
#include "camera.h"

/*
 * main menu specific behaviors since it's
 * meant to be somewhat interactive, but not necessarily
 * the same as the "ingame" stuff.

 *
 * The main menu hub is kind of intended to be a "hub" area
 * that grows a little after each stage.
 *
 * Just a nice thing to add a little world building.
 *
 */

struct MainMenu_Data;

/*
  NOTE: all pets will basically do the same thing on the
  main menu, since they're solely designed to be "flavor" objects.

  Unfortunately the reality of programming games or anything means that even
  "small" things take probably more code than anyone would like to ever admit.
*/
enum MainMenu_Pet_Action_Type {
    /* Three Idle Animations at most */
    MAIN_MENU_PET_ACTION_IDLE       = 0,
    MAIN_MENU_PET_ACTION_IDLE1      = 1,
    MAIN_MENU_PET_ACTION_IDLE2      = 2,

    MAIN_MENU_PET_ACTION_MOVING     = 3,
    MAIN_MENU_PET_ACTION_MAKE_NOISE = 4,

    // Petting the animals will override all other actions.
    // also they should make a sound.
    // NOTE: I'm going to currently crash on these.
    MAIN_MENU_PET_ACTION_APPRECIATE_INTERACTION = 5,
};

#define MAIN_MENU_PET_INTERACTION_RADIUS_PX (64)
struct MainMenu_Pet {
    s8 type; /* Refer to Game_State_Pet_Type in game_state.h */
    u8 current_action = MAIN_MENU_PET_ACTION_IDLE;

    f32 action_timer;
    f32 velocity_speed;

    V2 position; // spawn randomly in the main menu
    V2 current_direction;
    V2 sprite_scale = V2(1, 1);

    void draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources);
    void update(MainMenu_Data* state, f32 dt);
    void decide_new_action(random_state* prng);

    // All pets will have a fixed rectangle size since we don't really need collision detection...
    rectangle_f32 get_rect(void);
};

struct MainMenu_Player {
    bool visible = false;
    V2 position;
    V2 scale;
    V2 velocity;
    void draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources);
    void update(MainMenu_Data* state, f32 dt);
    rectangle_f32 get_rect();
};

struct MainMenu_Stage_Portal {
    bool visible = false;
    // safe to serialize data
    s32 stage_id;
    s32 prerequisites[16]; // -1 is bad
    V2  position;
    V2  scale;

    // runtime data
    bool triggered_level_selection;

    Particle_Emitter emitter_main;
    Particle_Emitter emitter_vortex;

    void draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources);
    rectangle_f32 get_rect();
};

struct MainMenu_Completed_MainGame_Cutscene_Data {
    bool triggered        = false;
    s32  phase            = 0;
};

struct MainMenu_Introduction_Cutscene_Data {
    bool triggered        = false;
    s32  phase            = 0;
};

#define MAX_MAINMENU_OUTERSPACE_STARS (2048)

enum MainMenu_ScreenMessage_Phase {
    MAIN_MENU_SCREEN_MESSAGE_APPEAR,
    MAIN_MENU_SCREEN_MESSAGE_WAIT_FOR_CONTINUE,
    MAIN_MENU_SCREEN_MESSAGE_DISAPPEAR,
};
struct MainMenu_ScreenMessage {
    s32    phase;
    f32    timer;
    string text;
};

struct MainMenu_Data {
    Particle_Pool particle_pool;

    MainMenu_Completed_MainGame_Cutscene_Data cutscene1;
    MainMenu_Introduction_Cutscene_Data       cutscene2;

    Fixed_Array<MainMenu_ScreenMessage> screen_messages;
    // For the background allowing you to read the messages.
    f32                                 screen_message_fade_t;
    
    // For now I'll just use a basic outerspacy sort of theme.
    V2 star_positions[2][MAX_MAINMENU_OUTERSPACE_STARS];

    struct camera   main_camera;
    MainMenu_Player player;
    MainMenu_Pet    pets[3];

    random_state prng;

    // NOTE: rename later.
    s32 stage_id_level_select = -1;
    s32 stage_id_level_in_stage_select = -1;

    Fixed_Array<MainMenu_Stage_Portal> portals;

    // runtime data
    // for the camera focus
    MainMenu_Stage_Portal* last_focus_portal = nullptr;

    void start_completed_maingame_cutscene(Game_State* game_state);
    void start_introduction_cutscene(Game_State* game_state, bool fasttrack);
    
    void update_and_render_cutscene1(struct render_commands* game_commands, struct render_commands* ui_commands, f32 dt);
    bool cutscene_active();

    bool screen_messages_finished();
    void screen_message_add(string message);
};

#endif
