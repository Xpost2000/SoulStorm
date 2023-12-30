/*
 * structure definitions for resource holder and main game state.
 */
#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "common.h"

#include "particle_system.h"
#include "entity.h"
#include "fixed_array.h"
#include "prng.h"
#include "camera.h"
#include "audio.h"

#include "game_task_scheduler.h"
#include "game_ui.h"

#define MAX_PICKUP_ENTITIES     (10000)
#define MAX_BULLETS             (3000)
#define MAX_SCORE_NOTIFICATIONS (5000)
#define MAX_ENEMIES             (128)
#define MAX_EXPLOSION_HAZARDS   (32)
#define MAX_LASER_HAZARDS       (32)
#define MAX_SCRIPTABLE_RENDER_OBJECTS (1000)
#define MAX_TRACKED_SCRIPT_LOADABLE_IMAGES (64)
#define MAX_TRACKED_SCRIPT_LOADABLE_SOUNDS (64)

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

enum Projectile_Sprite_Type {
    PROJECTILE_SPRITE_BLUE,
    PROJECTILE_SPRITE_BLUE_STROBING,
    PROJECTILE_SPRITE_BLUE_ELECTRIC,

    PROJECTILE_SPRITE_RED,
    PROJECTILE_SPRITE_RED_STROBING,
    PROJECTILE_SPRITE_RED_ELECTRIC,

    PROJECTILE_SPRITE_NEGATIVE,
    PROJECTILE_SPRITE_NEGATIVE_STROBING,
    PROJECTILE_SPRITE_NEGATIVE_ELECTRIC,

    PROJECTILE_SPRITE_HOT_PINK,
    PROJECTILE_SPRITE_HOT_PINK_STROBING,
    PROJECTILE_SPRITE_HOT_PINK_ELECTRIC,

    PROJECTILE_SPRITE_GREEN,
    PROJECTILE_SPRITE_GREEN_STROBING,
    PROJECTILE_SPRITE_GREEN_ELECTRIC,

    PROJECTILE_SPRITE_BLUE_DISK,
    PROJECTILE_SPRITE_RED_DISK,
    PROJECTILE_SPRITE_HOT_PINK_DISK,
    PROJECTILE_SPRITE_NEGATIVE_DISK,
    PROJECTILE_SPRITE_GREEN_DISK,

    /*
     * NOTE: this is technically only being used for the "visual" parts
     *
     * however, I'm sorely lacking in projectile variety so I'm gladly going to
     * reuse this if I can.
     */
    PROJECTILE_SPRITE_SPARKLING_STAR,

    PROJECTILE_SPRITE_TYPES
};

local int projectile_sprites_requiring_rotation[] = {
    PROJECTILE_SPRITE_BLUE_DISK,
    PROJECTILE_SPRITE_RED_DISK,
    PROJECTILE_SPRITE_HOT_PINK_DISK,
    PROJECTILE_SPRITE_NEGATIVE_DISK,
    PROJECTILE_SPRITE_GREEN_DISK
};

#define PLAY_AREA_WIDTH_PX (375)

// this is where the actual member variables of the game go
// the class is just a small wrapper for this stuff
enum Play_Area_Edge_Behavior {
    PLAY_AREA_EDGE_BLOCKING    = 0,
    PLAY_AREA_EDGE_DEADLY      = 1,
    PLAY_AREA_EDGE_WRAPPING    = 2,
    PLAY_AREA_EDGE_PASSTHROUGH = 3,
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
    GAME_SCREEN_COUNT     = 5,

/* #ifndef RELEASE */
/*     GAME_SCREEN_DEFAULT_MODE = GAME_SCREEN_TITLE_SCREEN, */
/* #else */
    GAME_SCREEN_DEFAULT_MODE = GAME_SCREEN_OPENING,
/* #endif */
};

#include "stages.h"
#include "stage.h"

enum Game_State_Pet_Type {
    GAME_PET_ID_NONE  = -1,
    GAME_PET_ID_CAT   = 0,
    GAME_PET_ID_DOG   = 1,
    GAME_PET_ID_FISH  = 2,
    GAME_PET_ID_COUNT = 3,
};

#include "title_screen_mode.h"
#include "main_menu_mode.h"
#include "opening_mode.h"

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

#define MAX_BASE_TRIES (5)

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

enum Boss_Healthbar_Animation_Display {
    BOSS_HEALTHBAR_ANIMATION_DISPLAY_SPAWN,
    BOSS_HEALTHBAR_ANIMATION_DISPLAY_IDLE,
    BOSS_HEALTHBAR_ANIMATION_DISPLAY_FALL_INTO_ORDER,
    BOSS_HEALTHBAR_ANIMATION_DISPLAY_DESPAWN,
    BOSS_HEALTHBAR_ANIMATION_DISPLAY_COUNT,
};

struct Boss_Healthbar_Display {
    s32    animation_state = BOSS_HEALTHBAR_ANIMATION_DISPLAY_SPAWN;
    u64    entity_uid      = 0;
    char  bossnamebuffer[64];
    // NOTE: position is relative to the Boss_Healthbar_Displays
    V2     position        = V2(0, 0);
    f32    alpha           = 0.0f;
    f32    animation_t     = 0.0f; // used when spawning in, or removal.
    V2     start_position_target;
    V2     end_position_target;
};

struct Boss_Healthbar_Displays {
    V2 position;
    Fixed_Array<Boss_Healthbar_Display> displays;

    void add(u64 entity_uid, string name);
    void remove(u64 entity_uid);

    void update(Game_State* state, f32 dt);
    void render(struct render_commands* ui_commands, Game_State* state);

    V2 element_position_for(s32 idx);
};

/*
  These are only used by the ingame background drawing task to
  generate background visuals.
*/
enum Scriptable_Render_Object_Layer {
    SCRIPTABLE_RENDER_OBJECT_LAYER_BACKGROUND = 0,
    SCRIPTABLE_RENDER_OBJECT_LAYER_FOREGROUND = 1,
    
};

// NOTE: these are updated entirely through
//       lua
struct Scriptable_Render_Object {
    s32 layer = SCRIPTABLE_RENDER_OBJECT_LAYER_BACKGROUND;

    struct image_id image_id      = {0};
    V2              position      = V2(0, 0);
    V2              scale           = V2(1, 1);
    V2              rotation_center = V2(0,0);
    rectangle_f32   src_rect      = RECTANGLE_F32_NULL;
    color32u8       modulation    = color32u8(255,255,255,255);
    s32             z_angle       = 0;
    s32             y_angle       = 0;
    s32             x_angle       = 0;

    void render(Game_Resources* resources, struct render_commands* render_commands);
};

#include "demo_recording.h"

enum Difficulty_Modifier_ID {
    DIFFICULTY_NORMAL  = 0,
    DIFFICULTY_EASY    = 1,
    DIFFICULTY_HARD    = 2,
    DIFFICULTY_HARDEST = 3,
};
struct Gameplay_Data_Pet_Information {
    string name;
    string description;
    s8     difficulty_modifier; // NOTE: does not inherently do anything. Only used in level scripts.
    s8     maximum_lives;
    s8     attack_pattern_id; // this will be checked in the player code for things.
    f32    score_modifier;
    // Not really sure how much I want this
    // but I'll have it here anyway.
    f32    speed_modifier;
};

struct Gameplay_Data {
    bool stage_completed;
    bool playing_practice_mode;
    Stage_State stage_state;
    Particle_Pool particle_pool;
    // TODO: allow player name configuration
    /* char          player_name[64]; */
    void build_current_input_packet();

    Gameplay_Frame_Input_Packet current_input_packet;
    Gameplay_Demo_Viewer        demo_viewer;
    Gameplay_Demo_Collection_UI demo_collection_ui;
    Gameplay_Recording_File     recording;

    Fixed_Array<Bullet>                   to_create_player_bullets;
    Fixed_Array<Bullet>                   to_create_enemy_bullets;
    Fixed_Array<Enemy_Entity>             to_create_enemies;
    Fixed_Array<Pickup_Entity>            to_create_pickups;

    Fixed_Array<Particle_Emitter> particle_emitters;
    Fixed_Array<Pickup_Entity>    pickups;
    Fixed_Array<Bullet>           bullets;
    Fixed_Array<Enemy_Entity>     enemies;
    Fixed_Array<Laser_Hazard>     laser_hazards;
    Fixed_Array<Explosion_Hazard> explosion_hazards;

    // NOTE: these are per frame.
    Fixed_Array<Scriptable_Render_Object> scriptable_render_objects;

    // TODO: adjust the position of these items.
    Fixed_Array<Gameplay_UI_Score_Notification> score_notifications;
    Fixed_Array<Gameplay_UI_Hitmark_Score_Notification> hit_score_notifications;
    Player              player;

    Boss_Healthbar_Displays boss_health_displays;
    Play_Area play_area;

    random_state prng;

    // this prng is not replicated. Used for currently only the dialogue system.
    // as it's data is not saved.
    random_state prng_unessential;

    camera       main_camera;

    bool paused_from_death = false;
    bool triggered_stage_completion_cutscene = false;
    bool queue_bomb_use = false;

    f32 fixed_tickrate_timer     = 0.0f;
    f32 fixed_tickrate_remainder = 0.0f;
    Gameplay_Stage_Introduction_Sequence   intro;
    Gameplay_Stage_Complete_Stage_Sequence complete_stage;

    s32 selected_pet = GAME_PET_ID_NONE;
    s32 unlocked_pets = 0; // [0,3]
    s32 tries = MAX_BASE_TRIES;
    s32 current_score = 0;
    f32 current_stage_timer = 0;

    Fixed_Array<image_id> script_loaded_images;
    Fixed_Array<Audio::Sound_ID> script_loaded_sounds;

    void unload_all_script_loaded_resources(Game_State* state, Game_Resources* resources);

    // NOTE:
    // Bullets and enemy entities are queued up because
    // they are multithreaded.
    void add_bullet(Bullet b);
    void add_pickup(Pickup_Entity pe);
    void add_laser_hazard(Laser_Hazard h);
    void add_explosion_hazard(Explosion_Hazard h);
    void add_enemy_entity(Enemy_Entity e);
    void add_pickup_entity(Pickup_Entity s);

    void add_scriptable_render_object(Scriptable_Render_Object ro);
    void update_and_render_all_background_scriptable_render_objects(Game_Resources* resources, struct render_commands* render_commands, f32 dt);
    void update_and_render_all_foreground_scriptable_render_objects(Game_Resources* resources, struct render_commands* render_commands, f32 dt);

    Enemy_Entity*  lookup_enemy(u64 uid);
    Bullet* lookup_bullet(u64 uid);

    bool entity_spawned(u64 uid);
    bool bullet_spawned(u64 uid);

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
    UI_STATE_PET_SELECT,
    UI_STATE_DEAD_MAYBE_RETRY,
    UI_STATE_ACHIEVEMENTS,
    UI_STATE_REPLAY_COLLECTION,
    UI_STATE_REPLAY_ASK_TO_SAVE,
    UI_STATE_CONFIRM_BACK_TO_MAIN_MENU,
    UI_STATE_CONFIRM_EXIT_TO_WINDOWS,
    UI_STATE_COUNT,
};

struct Achievement_Menu_Data {
    s32 page;
};

struct lua_State;


/*
  This is purely visual flare cause I think it would look cool,
  and I have a dual sense.

  Unfortunately it's difficult to think of genuinely super cool effects to
  do with the controller, so it's just interpolating the lightbar...
*/
struct Controller_LED_State {
    color32u8 primary_color = color32u8(255, 255, 255, 255);
    color32u8 target_color  = color32u8(255, 255, 255, 255);

    f32  animation_t         = 0.0f;
    f32  animation_max_t     = 1.0f; // to avoid initial division by 0.
    bool fade_back_when_done = true;
    u8   fade_phase          = 0;
    bool can_override        = true;
    bool finished_anim       = true;
    // data based API.
};

/*
 * This dialogue system is designed to work well with coroutine style
 * systems
 */
#define DIALOGUE_TYPE_SPEED (0.0325f)
enum Dialogue_Speaker_Animation_Type {
    DIALOGUE_SPEAKER_ANIMATION_NONE      = 0,      // use for introduction
    DIALOGUE_SPEAKER_ANIMATION_FADE_IN   = 1,   // use for introduction
    DIALOGUE_SPEAKER_ANIMATION_FADE_OUT  = 2,  // use for outro

    DIALOGUE_SPEAKER_ANIMATION_SLIDE_IN  = 3,  // use for introduction
    DIALOGUE_SPEAKER_ANIMATION_SLIDE_OUT = 4, // use for outro

    // NOTE: focus in and focus out
    //       will set their own animation state
    //
    DIALOGUE_SPEAKER_ANIMATION_FOCUS_OUT = 5, // darken, and shrink a little.
    DIALOGUE_SPEAKER_ANIMATION_FOCUS_IN  = 6,  // inverse of the above.

    DIALOGUE_SPEAKER_ANIMATION_SLIDE_FADE_IN  = 7,  // use for introduction
    DIALOGUE_SPEAKER_ANIMATION_SLIDE_FADE_OUT  = 8,  // use for introduction

    DIALOGUE_SPEAKER_ANIMATION_SHAKE     = 9,     // param: amount of shakes, intensity
    DIALOGUE_SPEAKER_ANIMATION_JUMP      = 10,      // param: amount of jumps, intensity
    DIALOGUE_SPEAKER_ANIMATION_COUNT     = 11,
};
struct Dialogue_Speaker_Animation {
    u8 type;

    s32 times;  // for shaking and jumping.
    s32 param0; // height / intensity;

    f32 t;
    f32 max_t;

    V2 shake_offset;
};
struct Dialogue_Speaker {
    Dialogue_Speaker_Animation animation;
    // Depends on how they're drawn
    // that the scale might have to be changed.
    image_id image;

    V2 offset_position = V2(0, 0);
    V2 image_scale = V2(1, 1);
    // NOTE: in units of 0.0 - 1.0
    // different "mood" images should be in the same resolution
    // so that nothing breaks.
    color32f32 modulation = color32f32(1, 1, 1, 1);
    bool mirrored = false;
    bool visible  = false;
};
struct Dialogue_State {
    bool in_conversation = false;
    bool confirm_continue = false;
    // Only two characters would ever really be speaking.
    Dialogue_Speaker speakers[2];
    char             current_line[256];

    int  shown_characters = 0;  // for typed out text
    int  length           = 0; 
    f32  type_timer       = 0.0f;

    int      tracked_image_count = 0;
    image_id tracked_images[512];
    /* Fixed_Array<image_id>        tracked_images; */
    /* Fixed_Array<Audio::Sound_ID> tracked_sounds; */
};

struct Game_State {
    s32 screen_mode      = GAME_SCREEN_DEFAULT_MODE;
    s32 last_screen_mode = GAME_SCREEN_DEFAULT_MODE;
    s32 ui_state         = UI_STATE_INACTIVE;
    s32 last_ui_state    = UI_STATE_INACTIVE;
    s32 last_completion_state = -1;

    Controller_LED_State led_state;

    Gameplay_Data    gameplay_data;
    MainMenu_Data    mainmenu_data;
    TitleScreen_Data titlescreen_data;
    OpeningMode_Data opening_data;
    Achievement_Menu_Data achievement_menu;

    Dialogue_State dialogue_state;

    Game_Task_Scheduler coroutine_tasks;
    Game_Resources* resources;

    lua_State* alloc_lua_bindings();

    void set_led_primary_color(color32u8 color);
    void set_led_target_color_anim(color32u8 color, f32 anim_length, bool overridable=true, bool fade_back_when_done=true);
    void set_led_target_color_anim_force(color32u8 color, f32 anim_length, bool overridable=true, bool fade_back_when_done=true);

    void kill_all_bullets();
    void kill_all_enemies();
    void convert_bullets_to_score_pickups(float radius=9999);
    void convert_enemies_to_score_pickups(float radius=9999);
};

inline static Game_State* lua_binding_get_gamestate(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    lua_remove(L, lua_gettop(L));
    return state;
}

enum Trophy_Icon_Frame {
    TROPHY_ICON_BLANK  = 0,
    TROPHY_ICON_SHINE1 = 1,
    TROPHY_ICON_SHINE2 = 2,
    TROPHY_ICON_SHINE3 = 3,
    TROPHY_ICON_SHINE4 = 4,
    TROPHY_ICON_SHINE5 = 5,
    TROPHY_ICON_COUNT  = 6,
};

struct Game_Resources {
    struct graphics_assets graphics_assets;
    font_id         menu_fonts[MENU_FONT_COUNT];
    image_id        circle;

    sprite_id       projectile_sprites[PROJECTILE_SPRITE_TYPES];
    sprite_id       player_sprite;
    sprite_id       circle_sprite;


    image_id        ui_marquee_bkrnd;
    image_id        ui_vignette_borders[2];

    GameUI_Ninepatch ui_chunky;
    GameUI_Ninepatch ui_chunky_outline;

    image_id         trophies_locked[TROPHY_ICON_COUNT];
    image_id         trophies_unlocked[TROPHY_ICON_COUNT];
    sprite_id        locked_trophy_sprite;
    sprite_id        unlocked_trophy_sprite;
    // Only one of these needs to exist since all of them will animate at the same rate.
    Sprite_Instance  locked_trophy_sprite_instance;
    Sprite_Instance  unlocked_trophy_sprite_instance;
    Texture_Atlas    ui_texture_atlas;

    // I should have more of these...
    Audio::Sound_ID        attack_sounds[2];
    Audio::Sound_ID        hit_sounds[2];

    // weird special case for bullets since those actually
    // require rotation.
    //
    // It's being used by entity rendering as a hint basically.
    inline bool sprite_id_should_be_rotated(sprite_id id) {
        for (int i = 0; i < array_count(projectile_sprites_requiring_rotation); ++i) {
            auto projectile_sprite_id = projectile_sprites[projectile_sprites_requiring_rotation[i]];
            if (projectile_sprite_id.index == id.index) {
                return true;
            }
        }

        return false;
    }

    inline Audio::Sound_ID random_attack_sound(struct random_state* prng) {
        return attack_sounds[
            random_ranged_integer(prng, 0, array_count(attack_sounds)-1)
        ];
    }

    inline Audio::Sound_ID random_hit_sound(struct random_state* prng) {
        return hit_sounds[
            random_ranged_integer(prng, 0, array_count(hit_sounds)-1)
        ];
    }

    inline font_cache* get_font(s32 variation) {
        struct font_cache* font = graphics_assets_get_font_by_id(&graphics_assets, menu_fonts[variation]);
        return font;
    }
};

Gameplay_Data_Pet_Information* game_get_pet_data(s32 id);

#endif
