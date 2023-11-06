/*
 * structure definitions for resource holder and main game state.
 */
#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "common.h"

#include "entity.h"
#include "fixed_array.h"
#include "prng.h"
#include "camera.h"

#include "game_task_scheduler.h"

#define MAX_BULLETS           (10000)
#define MAX_EXPLOSION_HAZARDS (1024)
#define MAX_LASER_HAZARDS     (1024)
#define MAX_ENEMIES           (1024)
#define MAX_SCORE_NOTIFICATIONS (4096)

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
    PLAY_AREA_EDGE_PASSTHROUGH,
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
    bool is_inside_logical(rectangle_f32 rect) const;

    // this is for your mouse cursor or a "real" thing
    bool is_inside_absolute(rectangle_f32 rect) const;
};

enum Game_Screen_Modes {
    /*
     * A text exposition cutscene + my branding I guess
     */
    GAME_SCREEN_OPENING   = 0,

    /*
     * This solely exists for presentation
     * purposes, but you never see the title
     * outside of booting the game.
     */
    GAME_SCREEN_TITLE_SCREEN = 1,
    GAME_SCREEN_MAIN_MENU = 2,
    GAME_SCREEN_INGAME    = 3,

    /*
     * Not going to be prentenious like Legends, it'll just be a simple
     * flat text screen.
     *
     * Honestly, if I wanted to play with it more I could add some interactivity.
     */
    GAME_SCREEN_CREDITS   = 4,
    GAME_SCREEN_COUNT     = 5
};

#include "stages.h"
#include "stage.h"

#include "title_screen_mode.h"
#include "main_menu_mode.h"

// I do like using state machines for this kind of thing
// at least when you have to write code for animation.
enum Gameplay_Stage_Introduction_Sequence_Stage {
    GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_NONE,
    GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_FADE_IN,
    GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_TYPE_IN_NAME,
    GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_TYPE_IN_SUBTITLE,
    GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_LINGER,
    GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_FADE_OUT_EVERYTHING,
};
#define GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_TYPE_SPEED (0.05f) // I do want to type it in but for now. Later.
struct Gameplay_Stage_Introduction_Sequence {
    s32 stage = GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_FADE_IN;
    Timer stage_timer;

    f32 type_timer      = 0.0f;
    s32 name_length     = 0;
    s32 subtitle_length = 0;
};

// NOTE:
// I know this name looks horribly redundant, but my naming style will
// end up producing funny stuff like this every once in a while.
enum Gameplay_Stage_Complete_Stage_Sequence_Stage {
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE,
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_IN,
    // would like to count up the score visually but for now just to make it quick no.
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_SHOW_SCORE,
    // Maybe allow going to the next stage immediately? Probably not.
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_WAIT_UNTIL_FADE_OUT,
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_OUT,
};

struct Gameplay_Stage_Complete_Stage_Sequence {
    s32 stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE;
    Timer stage_timer;
};

#define MAX_BASE_TRIES (3)

// will do a small jump before dying.
// NOTE: not the same as the hit marker score notifications
#define GAMEPLAY_UI_SCORE_NOTIFICATION_DEFAULT_LIFETIME (0.25f)
#define GAMEPLAY_UI_SCORE_NOTIFICATION_RISE_AMOUNT      (65)
struct Gameplay_UI_Score_Notification {
    s32 additional_score;
    Timer lifetime = Timer(GAMEPLAY_UI_SCORE_NOTIFICATION_DEFAULT_LIFETIME);
};

struct Gameplay_UI_Hitmark_Score_Notification {
    V2 where;
    s32 additional_score;
    Timer lifetime = Timer(GAMEPLAY_UI_SCORE_NOTIFICATION_DEFAULT_LIFETIME);
};

struct Gameplay_Data {
    // As a coroutine, the stage will have to yield
    // and set this flag on it's own.
    // The only other thing I can do is just repeatedly
    // monitor coroutine task id, but that's dirty.
    // This is the least bad idea I could think of to handle
    // this, which is fine since it's really one thing.
    bool stage_completed;
    Stage_State stage_state;

    // NOTE:
    // to make this parallel safe, I need to
    // defer entity creation to after the existing entity updates so my
    // engine will not explode dangerously :)
    Fixed_Array<Bullet>           to_create_player_bullets;
    Fixed_Array<Bullet>           to_create_enemy_bullets;
    Fixed_Array<Enemy_Entity>     to_create_enemies;

    Fixed_Array<Bullet>           bullets;
    Fixed_Array<Enemy_Entity>     enemies;
    Fixed_Array<Laser_Hazard>     laser_hazards;
    Fixed_Array<Explosion_Hazard> explosion_hazards;

    Fixed_Array<Gameplay_UI_Score_Notification> score_notifications;
    Fixed_Array<Gameplay_UI_Hitmark_Score_Notification> hit_score_notifications;
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

    bool paused_from_death = false;
    bool triggered_stage_completion_cutscene = false;

    Gameplay_Stage_Introduction_Sequence   intro;
    Gameplay_Stage_Complete_Stage_Sequence complete_stage;

    s32 tries = MAX_BASE_TRIES;
    s32 current_score = 0;
    f32 auto_score_timer = 0;

    // NOTE:
    // Bullets and enemy entities are queued up because
    // they are multithreaded.
    void add_bullet(Bullet b);
    void add_laser_hazard(Laser_Hazard h);
    void add_explosion_hazard(Explosion_Hazard h);
    void add_enemy_entity(Enemy_Entity e);

    void reify_all_creation_queues();

    bool any_hazards() const;
    bool any_enemies() const;
    bool any_bullets() const;
    bool any_living_danger() const;

    void notify_score(s32 amount, bool interesting=false);
    // NOTE: is implicitly an "interesting" score.
    void notify_score_with_hitmarker(s32 amount, V2 where);
};

enum UI_State_Mode {
    UI_STATE_INACTIVE,
    UI_STATE_PAUSED,
    UI_STATE_OPTIONS,
    UI_STATE_STAGE_SELECT,
    UI_STATE_DEAD_MAYBE_RETRY,
    UI_STATE_ACHIEVEMENTS,
    UI_STATE_CONFIRM_BACK_TO_MAIN_MENU,
    UI_STATE_CONFIRM_EXIT_TO_WINDOWS,
    UI_STATE_COUNT,
};

struct Achievement_Menu_Data {
    s32 page;
};

struct Game_State {
    s32 screen_mode      = GAME_SCREEN_MAIN_MENU;
    s32 last_screen_mode = GAME_SCREEN_MAIN_MENU;
    s32 ui_state         = UI_STATE_INACTIVE;
    s32 last_ui_state    = UI_STATE_INACTIVE;

    Gameplay_Data    gameplay_data;
    MainMenu_Data    mainmenu_data;
    TitleScreen_Data titlescreen_data;
    Achievement_Menu_Data achievement_menu;

    Game_Task_Scheduler coroutine_tasks;
};

struct Game_Resources {
    struct graphics_assets graphics_assets;
    font_id         menu_fonts[MENU_FONT_COUNT];
    image_id        circle;

    inline font_cache* get_font(s32 variation) {
        struct font_cache* font = graphics_assets_get_font_by_id(&graphics_assets, menu_fonts[variation]);
        return font;
    }
};

// No methods, just a bunch of free functions if I have any here.

#endif
