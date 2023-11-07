#ifndef MAIN_MENU_MODE_H
#define MAIN_MENU_MODE_H

#include "v2.h"
#include "camera.h"

/*
 * main menu specific behaviors since it's
 * meant to be somewhat interactive, but not necessarily
 * the same as the "ingame" stuff.
 */

struct MainMenu_Data;

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

    void draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources);
    rectangle_f32 get_rect();
};

/*
  NOTE: While I do have a coroutine system currently, it only really
  works when stuff is reified, and none of the UI exists as an object.

  The few times I will use these similar cutscene sequences, I don't think is honestly
  worth the effort of creating an API to operate these cutscenes TBH.

  I'm only doing it for the game levels because those have a higher change frequency as opposed
  to something like this.

  (at least API access wise), so this sequence is implemented as a state
  machine.
*/
enum MainMenu_Completed_MainGame_Cutscene_Phase {
    MAINMENU_COMPLETED_MAINGAME_CUTSCENE_NONE,
    MAINMENU_COMPLETED_MAINGAME_CUTSCENE_FADE_IN,
    MAINMENU_COMPLETED_MAINGAME_CUTSCENE_MESSAGE1,
    MAINMENU_COMPLETED_MAINGAME_CUTSCENE_FOCUS_ON_POSTGAME_PORTAL,
    MAINMENU_COMPLETED_MAINGAME_CUTSCENE_SPAWN_POSTGAME_PORTAL,
    MAINMENU_COMPLETED_MAINGAME_CUTSCENE_UNFOCUS,
    MAINMENU_COMPLETED_MAINGAME_CUTSCENE_MESSAGE2,
    MAINMENU_COMPLETED_MAINGAME_CUTSCENE_FADE_OUT,
    //bye
};

struct MainMenu_Completed_MainGame_Cutscene_Data {
    bool triggered        = false;
    s32  phase            = MAINMENU_COMPLETED_MAINGAME_CUTSCENE_NONE;
    s32  characters_shown = 0;
    f32  type_timer       = 0.0f;
    f32  timer            = 0.0f;
};

enum MainMenu_Introduction_Cutscene_FirstTime_Phase {
    MAIN_MENU_INTRODUCTION_CUTSCENE_FIRSTTIME_NONE,
    MAIN_MENU_INTRODUCTION_CUTSCENE_FIRSTTIME_START, // from fade out presumably.
    MAIN_MENU_INTRODUCTION_CUTSCENE_FIRSTTIME_ZOOM_IN,
    MAIN_MENU_INTRODUCTION_CUTSCENE_FIRSTTIME_BUILD_UP,
    MAIN_MENU_INTRODUCTION_CUTSCENE_FIRSTTIME_MESSAGE1,
    MAIN_MENU_INTRODUCTION_CUTSCENE_FIRSTTIME_SPAWN_PLAYER,
    MAIN_MENU_INTRODUCTION_CUTSCENE_FIRSTTIME_MESSAGE2,
    // maybe zoom into each portal?
    MAIN_MENU_INTRODUCTION_CUTSCENE_FIRSTTIME_FADE_OUT_MESSAGE,
    // maybe player animation?
    MAIN_MENU_INTRODUCTION_CUTSCENE_FIRSTTIME_FADE_OUT_REST,
    // bye
};

enum MainMenu_Introduction_Cutscene_Phase {
    MAIN_MENU_INTRODUCTION_CUTSCENE_NONE,
    MAIN_MENU_INTRODUCTION_CUTSCENE_BUILD_UP,
    MAIN_MENU_INTRODUCTION_CUTSCENE_SPAWN_PLAYER,
};

struct MainMenu_Introduction_Cutscene_Data {
    bool triggered        = false;
    s32  phase            = 0;
    bool first_time_load  = true;
    s32  characters_shown = 0;
    f32  timer            = 0.0f;
    f32  type_timer       = 0.0f;
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
    MainMenu_Completed_MainGame_Cutscene_Data cutscene1;
    MainMenu_Introduction_Cutscene_Data       cutscene2;

    Fixed_Array<MainMenu_ScreenMessage> screen_messages;
    // For the background allowing you to read the messages.
    f32                                 screen_message_fade_t;
    
    // For now I'll just use a basic outerspacy sort of theme.
    V2 star_positions[2][MAX_MAINMENU_OUTERSPACE_STARS];

    struct camera   main_camera;
    MainMenu_Player player;

    random_state prng;

    // NOTE: rename later.
    s32 stage_id_level_select = -1;
    s32 stage_id_level_in_stage_select = -1;

    Fixed_Array<MainMenu_Stage_Portal> portals;

    // runtime data
    // for the camera focus
    MainMenu_Stage_Portal* last_focus_portal = nullptr;

    void start_completed_maingame_cutscene();
    void start_introduction_cutscene(bool fasttrack);
    
    void update_and_render_cutscene1(struct render_commands* game_commands, struct render_commands* ui_commands, f32 dt);

    // These **could** be rewritten as coroutines but
    // I'm more used to doing this, and they're not being touched often
    // enough to require the usage of the scheduler.
    // If I really feel it. I can rewrite it, but I'd rather just write it the
    // way I actually know how to do reliably first :)
    void update_and_render_cutscene2(struct render_commands* game_commands, struct render_commands* ui_commands, f32 dt);
    void update_and_render_cutscene2_firsttime(struct render_commands* game_commands, struct render_commands* ui_commands, f32 dt);
    void update_and_render_cutscene2_fasttrack(struct render_commands* game_commands, struct render_commands* ui_commands, f32 dt);
    bool cutscene_active();

    bool screen_messages_finished();
    void screen_message_add(string message);
};

#endif
