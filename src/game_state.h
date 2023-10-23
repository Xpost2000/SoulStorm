/*
 * structure definitions for resource holder and main game state.
 */
#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "entity.h"
#include "fixed_array.h"
#include "prng.h"
#include "camera.h"

static string menu_font_variation_string_names[] = {
    string_literal("res/fonts/gnsh-bitmapfont-colour1.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour2.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour3.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour4.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour5.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour6.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour7.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour8.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour9.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour10.png"),
};
/* using GNSH fonts, which are public domain, but credits to open game art, this font looks cool */
enum menu_font_variation {
    MENU_FONT_COLOR_GOLD,

    MENU_FONT_COLOR_ORANGE,
    MENU_FONT_COLOR_LIME,
    MENU_FONT_COLOR_SKYBLUE,
    MENU_FONT_COLOR_PURPLE,
    MENU_FONT_COLOR_BLUE,
    MENU_FONT_COLOR_STEEL,
    MENU_FONT_COLOR_WHITE,
    MENU_FONT_COLOR_YELLOW,
    MENU_FONT_COLOR_BLOODRED,

    /* I want room to have more fonts though, although GNSH fonts are very very nice. */
    MENU_FONT_COUNT,
};

#define PLAY_AREA_WIDTH_PX (350)

// this is where the actual member variables of the game go
// the class is just a small wrapper for this stuff
enum Play_Area_Edge_Behavior {
    PLAY_AREA_EDGE_BLOCKING = 0,
    PLAY_AREA_EDGE_DEADLY,
    PLAY_AREA_EDGE_WRAPPING,
    PLAY_AREA_EDGE_BEHAVIOR_COUNT,
};
struct Play_Area {
    int x      = 0;
    int width  = PLAY_AREA_WIDTH_PX;
    int height = 480;

    union {
        struct {
            u8 edge_behavior_top, edge_behavior_bottom, edge_behavior_left, edge_behavior_right;
        };
        u8 edge_behaviors[4];
    };

    void set_all_edge_behaviors_to(u8 value);

    // this is for the "game objects"
    bool is_inside_logical(rectangle_f32 rect);

    // this is for your mouse cursor or a "real" thing
    bool is_inside_absolute(rectangle_f32 rect);
};

enum Game_Screen_Modes {
    GAME_SCREEN_OPENING   = 0,
    GAME_SCREEN_MAIN_MENU = 1,
    GAME_SCREEN_INGAME    = 2,
    GAME_SCREEN_CREDITS   = 3,
    GAME_SCREEN_COUNT     = 4
};

#include "stages.h"
#include "main_menu_mode.h"

struct Gameplay_Data {
    Fixed_Array<Bullet>           bullets;
    Fixed_Array<Laser_Hazard>     laser_hazards;
    Fixed_Array<Explosion_Hazard> explosion_hazards;
    // should be in a separate list of it's own...
    Player              player;

    // should also have some form of entity iterator
    // or something later.

    // NOTE: the game runs in a "logical" resolution of (w)x480?
    // so I'll pick a play area that's generally okay for the base configuration...
    Play_Area play_area;

    // The camera is technically "static",
    // or really static. This is really only here so I can get working
    // screen shake behavior that looks cool.
    // I'll fake the illusion of movement through the background primarily
    random_state prng;
    camera       main_camera;
};
struct Game_State {
    s32 screen_mode = GAME_SCREEN_MAIN_MENU;
    Gameplay_Data gameplay_data;
    MainMenu_Data mainmenu_data;
    bool paused;
};

struct Game_Resources {
    graphics_assets graphics_assets;
    font_id         menu_fonts[MENU_FONT_COUNT];
    image_id        circle;

    inline font_cache* get_font(s32 variation) {
        struct font_cache* font = graphics_assets_get_font_by_id(&graphics_assets, menu_fonts[variation]);
        return font;
    }
};

// No methods, just a bunch of free functions if I have any here.

#endif
