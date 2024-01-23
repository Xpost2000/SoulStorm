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
    MAIN_MENU_PET_ACTION_POOP       = 5,
    MAIN_MENU_PET_MAIN_ACTIONS      = 6,

    // Petting the animals will override all other actions.
    // also they should make a sound.
    // NOTE: I'm going to currently crash on these.
    MAIN_MENU_PET_ACTION_APPRECIATE_INTERACTION = 7,
};

#define MAIN_MENU_PET_INTERACTION_RADIUS_PX (64)
struct MainMenu_Pet {
    s8 type; /* Refer to Game_State_Pet_Type in game_state.h */
    u8 current_action = MAIN_MENU_PET_ACTION_IDLE;

    f32 action_timer;

    V2 position; // spawn randomly in the main menu
    V2 current_direction;
    V2 sprite_scale = V2(1, 1);

    void draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources);
    void update(MainMenu_Data* state, f32 dt);
    void decide_new_action(random_state* prng);

    // All pets will have a fixed rectangle size since we don't really need collision detection...
    rectangle_f32 get_rect(void);
};

enum MainMenu_Player_Facing_Direction {
    MAIN_MENU_PLAYER_FACING_DIRECTION_FORWARD = 0, // towards screen
    MAIN_MENU_PLAYER_FACING_DIRECTION_BACK = 1,
    MAIN_MENU_PLAYER_FACING_DIRECTION_LEFT = 2,
    MAIN_MENU_PLAYER_FACING_DIRECTION_RIGHT = 3,
    MAIN_MENU_PLAYER_FACING_DIRECTION_COUNT = 4,
};

struct MainMenu_Player {
    u8 facing_direction;
    bool visible = false;
    V2 position;
    V2 scale;
    V2 velocity;
    void draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources);
    void update(MainMenu_Data* state, f32 dt);
    rectangle_f32 get_rect();
};

#define MAIN_MENU_STAGE_PORTAL_TIME_UNTIL_NEXT_FRAME (0.100f)
struct MainMenu_Stage_Portal {
    bool visible = false;
    // safe to serialize data
    s32 stage_id;
    s32 prerequisites[16]; // -1 is bad
    V2  position;
    V2  scale;

    s32 frame_index = 0;
    f32 animation_t = 0.0f;

    // runtime data
    bool triggered_level_selection;

    Particle_Emitter emitter_main;
    Particle_Emitter emitter_vortex;

    void draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources);
    void update(MainMenu_Data* const state, f32 dt);
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

enum MainMenu_Unlock_Pet_Cutscene_Phase {
    MAIN_MENU_UNLOCK_PET_CUTSCENE_PHASE_OFF           = 0,
    MAIN_MENU_UNLOCK_PET_CUTSCENE_COROUTINE_TASK      = 1,
    MAIN_MENU_UNLOCK_PET_CUTSCENE_FADE_IN_UNLOCK_BOX  = 2,
    MAIN_MENU_UNLOCK_PET_CUTSCENE_POP_IN_PET          = 3, // This is a white flash.
    MAIN_MENU_UNLOCK_PET_CUTSCENE_IDLE                = 4,
    MAIN_MENU_UNLOCK_PET_CUTSCENE_FADE_OUT_UNLOCK_BOX = 5,
};
struct MainMenu_Unlock_Pet_Cutscene_Data {
    bool triggered        = false;
    s32  phase            = 0;
    f32  timer            = 0;
};

#define MAX_MAINMENU_OUTERSPACE_STARS (2000)
#define MAX_MAINMENU_SPARKLING_STARS (64)

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

#define MAIN_MENU_POOP_LIFETIME (5.0f)

// This is just cause I think it would be more visually interesting to have
// on the main menu.
// All of the pets can poop. Some of the pets should have a more special action
// depending on what they are.
// Like you can throw a ball and the dog will try and catch it or whatever.
struct MainMenu_Clutter_Poop {
    V2  position;
    f32 lifetime = MAIN_MENU_POOP_LIFETIME;
    bool dead = false;

    void update(f32 dt);
    void draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources);
};

struct MainMenu_Data {
    Particle_Pool particle_pool;

    MainMenu_Completed_MainGame_Cutscene_Data cutscene1;
    MainMenu_Introduction_Cutscene_Data       cutscene2;
    MainMenu_Unlock_Pet_Cutscene_Data         cutscene3;

    Fixed_Array<MainMenu_ScreenMessage> screen_messages;
    Fixed_Array<MainMenu_Clutter_Poop>  clutter_poops;
    // For the background allowing you to read the messages.
    f32                                 screen_message_fade_t;
    
    // For now I'll just use a basic outerspacy sort of theme.
    V2 star_positions[2][MAX_MAINMENU_OUTERSPACE_STARS];
    Visual_Sparkling_Star_Data sparkling_stars[MAX_MAINMENU_SPARKLING_STARS];

    struct camera   main_camera;
    MainMenu_Player player;
    MainMenu_Pet    pets[3];

    random_state prng;

    // NOTE: rename later.
    s32 stage_id_level_select = -1;
    s32 stage_id_level_in_stage_select = -1;
    s32 stage_pet_selection = 0;

    Fixed_Array<MainMenu_Stage_Portal> portals;

    // runtime data
    // for the camera focus
    MainMenu_Stage_Portal* last_focus_portal = nullptr;

    void spawn_poop(V2 where);
    void cleanup_all_dead_poops(void);

    void start_completed_maingame_cutscene(Game_State* game_state);
    void start_unlock_pet_cutscene(Game_State* game_state);
    void start_introduction_cutscene(Game_State* game_state, bool fasttrack);
    
    void update_and_render_cutscene1(struct render_commands* game_commands, struct render_commands* ui_commands, f32 dt);
    void adjust_entities_for_screen_resolution(int screen_width, int screen_height);
    bool cutscene_active();

    bool screen_messages_finished();
    void screen_message_add(string message);

    // for camera sync
    int last_screen_width = -1;
    int last_screen_height = -1;
};

#endif
