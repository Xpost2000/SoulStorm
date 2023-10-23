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
    void update(MainMenu_Data* const state, f32 dt);
    rectangle_f32 get_rect();
};

struct MainMenu_Stage_Portal {
    s32 stage_id;
    s32 prerequisites[16];
    V2  position;
    V2  scale;
};

struct MainMenu_Data {
    struct camera   main_camera;
    MainMenu_Player player;

    Fixed_Array<MainMenu_Stage_Portal> portals;
};

#endif
