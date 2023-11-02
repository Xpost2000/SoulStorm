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
    V2 position;
    V2 scale;
    V2 velocity;
    void draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources);
    void update(MainMenu_Data* state, f32 dt);
    rectangle_f32 get_rect();
};

struct MainMenu_Stage_Portal {
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

#define MAX_MAINMENU_OUTERSPACE_STARS (2048)
struct MainMenu_Data {
    // For now I'll just use a basic outerspacy sort of theme.
    V2 star_positions[2][MAX_MAINMENU_OUTERSPACE_STARS];

    struct camera   main_camera;
    MainMenu_Player player;

    // NOTE: rename later.
    s32 stage_id_level_select = -1;
    s32 stage_id_level_in_stage_select = -1;

    Fixed_Array<MainMenu_Stage_Portal> portals;

    // runtime data
    // for the camera focus
    MainMenu_Stage_Portal* last_focus_portal = nullptr;

};

#endif
