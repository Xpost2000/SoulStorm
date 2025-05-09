/*
 * structure definitions for resource holder and main game state.

 NOTE: maybe spread some of this out a bit more.
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

#define MAX_PICKUP_ENTITIES                (12000)
#define MAX_BULLETS                        (7500)
#define MAX_SCORE_NOTIFICATIONS            (30000)
#define MAX_ENEMIES                        (512)
#define MAX_EXPLOSION_HAZARDS              (MAX_ENEMIES)
#define MAX_LASER_HAZARDS                  (256)
#define MAX_SCRIPTABLE_RENDER_OBJECTS      (512)
#define MAX_BACKGROUND_ENTITIES            (128)
#define MAX_TRACKED_SCRIPT_LOADABLE_IMAGES (128)
#define MAX_TRACKED_SCRIPT_LOADABLE_SOUNDS (128)

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

/* NOTE:
   I am probably leaving these constants hardcoded, even if I push
   it to be more data driven. (cause it's too late to change some things, or I'd rather not
   being doing it, since some of the game code is in C++ and some is in lua.)

   NOTE: these are the "assumed" default projectile sprites the game always assumes
   we have.

   Additional sprites are added through the manifest.lua file. Removing any of the
   original existing sprites will cause the game/engine to crash though or not work
   properly.
*/
enum Projectile_Sprite_Type {
    PROJECTILE_SPRITE_BLUE = 0,
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

    // maximum assumed allowed.
    PROJECTILE_SPRITE_TYPES = 256
};

enum Entity_Sprite_Type {
    // NOTE: there are no hardcoded entity sprites in the engine.
    ENTITY_SPRITE_TYPES = 128,
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

local string play_area_edge_behavior_strings[] = {
    string_literal("play-area-edge-blocking"),
    string_literal("play-area-edge-deadly"),
    string_literal("play-area-edge-wrapping"),
    string_literal("play-area-edge-passthrough"),
    string_literal("(count)"),
};

enum Play_Area_Edge_Id {
    PLAY_AREA_EDGE_ID_TOP = 0,
    PLAY_AREA_EDGE_ID_BOTTOM,
    PLAY_AREA_EDGE_ID_LEFT,
    PLAY_AREA_EDGE_ID_RIGHT,
    PLAY_AREA_EDGE_ID_COUNT,
};

local string play_area_edge_id_strings[] = {
    string_literal("play-area-edge-top"),
    string_literal("play-area-edge-bottom"),
    string_literal("play-area-edge-left"),
    string_literal("play-area-edge-right"),
    string_literal("(count)"),
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

    // Only one ending. Need to draw more assets for this one!
    // Probably just a cheesy congratulations screen.
    GAME_SCREEN_ENDING    = 5,

    // NOTE(jerry): needed to composite some scenes with the puppet
    GAME_SCREEN_TRAILER_CUTAWAY = 6,
    GAME_SCREEN_COUNT     = 7,

#ifdef BUILD_DEMO
    // The demo is always with the title screen
  GAME_SCREEN_DEFAULT_MODE = GAME_SCREEN_TITLE_SCREEN,
#else
#ifndef RELEASE
    // Testing purposes I want to skip the intro.
    GAME_SCREEN_DEFAULT_MODE = GAME_SCREEN_TITLE_SCREEN,
    //GAME_SCREEN_DEFAULT_MODE = GAME_SCREEN_ENDING,
    /* GAME_SCREEN_DEFAULT_MODE = GAME_SCREEN_OPENING, */
#else
    GAME_SCREEN_DEFAULT_MODE = GAME_SCREEN_OPENING,
    /* GAME_SCREEN_DEFAULT_MODE = GAME_SCREEN_ENDING, */
#endif
#endif
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

local string pet_type_strings[] = {
    string_literal("pet-id-none"),
    string_literal("pet-id-cat"),
    string_literal("pet-id-dog"),
    string_literal("pet-id-fish"),
    string_literal("(count)"),
};

// Used for title screen and main menu
struct Visual_Sparkling_Star_Data {
    bool hide = true;
    int frame_index; // manually animated (8 frames)
    f32 anim_timer;
    f32 visibility_delay_timer;
    f32 max_visibility_delay_timer;
    V2  position;
    f32 scale; /* [1.0, 1.5] */

    void update(f32 dt);
    void draw(struct render_commands* commands, Game_Resources* resources);
};

#include "title_screen_mode.h"
#include "main_menu_mode.h"
#include "opening_mode.h"
#include "ending_mode.h"
#include "trailer_cutaway_mode.h"

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

    void begin_sequence(void);
};

// NOTE:
// I know this name looks horribly redundant, but my naming style will
// end up producing funny stuff like this every once in a while.
enum Gameplay_Stage_Complete_Stage_Sequence_Stage {
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE,
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_ANIMATE_PLAYER_EXIT, // NOTE(jerry): skipped for replay mode. Only used in normal-play.
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_IN,
    // would like to count up the score visually but for now just to make it quick no.
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_SHOW_SCORE,
    // Maybe allow going to the next stage immediately? Probably not.
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_TALLY_TOTAL_SCORE,
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_WAIT_UNTIL_FADE_OUT,
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_OUT,
    GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_COUNT,
};

enum Gameplay_Stage_Complete_Stage_Player_Exit_Animation_Stage {
    GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_PIVOT_TO_CENTER,
    GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_BACK_UP,
    GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_BLAST_OFF,
    GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_LINGER,
    GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_DONE,
    GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_COUNT,
};

struct Gameplay_Stage_Complete_Stage_Sequence {
    s32 stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE;
    s32 player_exit_animation_stage = GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_PIVOT_TO_CENTER;
    Timer stage_timer;
    Timer exit_animation_stage_timer;
    Particle_Emitter player_propel_particles;

    void begin_sequence(bool replay_mode=false);
    void reset(void);
};

// NOTE: you can achieve this through various different things
// like maximizing your score.
#define MAX_TRIES_ALLOWED (10)
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
    Particle_Emitter* emitter = 0;
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
enum Attack_Pattern {
    // arc_pattern1 and arc_pattern1 but smaller.
    ATTACK_PATTERN_DEFAULT = 0,
    ATTACK_PATTERN_EXTRA1  = 1,
    ATTACK_PATTERN_EXTRA2  = 2,
    ATTACK_PATTERN_EXTRA3  = 3,
    ATTACK_PATTERN_COUNT   = 4
};
enum Bomb_Pattern {
    // clear screen, take a life. that sort of thing
    BOMB_PATTERN_DEFAULT = 0,
    BOMB_PATTERN_EXTRA1  = 1,
    BOMB_PATTERN_EXTRA2  = 2,
    BOMB_PATTERN_EXTRA3  = 3,
    BOMB_PATTERN_COUNT   = 4,
};

// NOTE: pets may choose to have their own
//       attack patterns but that's based on their
//       ID.
struct Gameplay_Data_Pet_Information {
    string name;
    string description;
    s8     difficulty_modifier; // NOTE: does not inherently do anything. Only used in level scripts.
    s8     maximum_lives;
    s8     attack_pattern_id; // this will be checked in the player code for things.
    s8     bomb_pattern_id;   // also checked in player code
    f32    score_modifier;
    // Not really sure how much I want this
    // but I'll have it here anyway.
    f32    speed_modifier;
};

enum Border_Flash_Id_Type {
    BORDER_FLASH_ID_TYPE_NONE        = 0,
    BORDER_FLASH_ID_TYPE_BLOCKING    = 1,
    BORDER_FLASH_ID_TYPE_DEADLY      = 2,
    BORDER_FLASH_ID_TYPE_WRAPPING    = 3,
    BORDER_FLASH_ID_TYPE_PASSTHROUGH = 4,
    BORDER_FLASH_ID_TYPE_ALERT       = 5,
    BORDER_FLASH_ID_TYPE_COUNT,
};

local string border_flash_id_strings[] = {
    string_literal("border-flash-none"),
    string_literal("border-flash-blocking"),
    string_literal("border-flash-deadly"),
    string_literal("border-flash-wrapping"),
    string_literal("border-flash-passthrough"),
    string_literal("border-flash-alert"),
    string_literal("(count)"),
};

#define BORDER_NOTIFY_FLASH_COUNT (16)
#define BORDER_NOTIFY_PER_FLASH_LENGTH (0.175f)
struct Border_Flash_Data {
    s32  flash_id_type;
    bool delay_between_flash;
    s32  flash_count;
    f32  per_flash_length; // NOTE(jerry): maybe not tuning anything?
};

#define POINTS_TO_AWARD_EXTRA_LIFE (165000)
enum Gameplay_Data_Particle_Spawn_Request_Type {
    GAMEPLAY_DATA_PARTICLE_SPAWN_REQUEST_TYPE_NONE,
    GAMEPLAY_DATA_PARTICLE_SPAWN_REQUEST_TYPE_LOST_LIFE,
    GAMEPLAY_DATA_PARTICLE_SPAWN_REQUEST_TYPE_GAINED_LIFE,
};
struct Gameplay_Data_Particle_Spawn_Request {
    u8 type;
    union {
        s32 data; // god knows what this means.
    };
};

struct Gameplay_Data_Gameplay_Alert {
  // NOTE(jerry):
  // always draw as center aligned, and only one alert
  // at a time, no queuing.
  u8 font_variation;
  bool enabled;
  f32 timer;
  f32 timer_max;
  //f32 time_per_blink;
  //bool blinked;
  char text[256];
};

struct Try_Continues_Data {
  f32 count_down;
  bool continued_once;
};

struct Gameplay_Data {
    bool campaign_perfect_clear;
    bool stage_perfect_clear;
    bool stage_completed;
    bool playing_practice_mode;
    bool manual_menu_quit_out;
    int  score_awarded_points = 0;
    int  extra_life_awarded_count = 0;
    
    // gameplay current stage statistics
    int  death_count = 0;
    int  burst_usage_count = 0;
    int  total_score = 0; // calculate after adjustments
    int  continue_count = 0;
    s32  total_run_score = 0;

    f32  focus_tint_fade_t = 0.0f;
    f32  focus_hitbox_fade_t = 0.0f;

    s32  invalid_usage_flash_count = 0;
    f32  invalid_usage_flash_t = 0;

    Gameplay_Data_Gameplay_Alert game_alert;

    bool allow_border_switch_flashing = true; // automatic border flashing behavior
    Border_Flash_Data border_flashes[4];
    void border_notify(s32 id, s32 type, int flash_count=BORDER_NOTIFY_FLASH_COUNT,  bool override=false);
    void border_stop_notify(s32 id);
    void border_stop_all_notifications(void);
    void border_notify_current_status(void);

    Try_Continues_Data try_continues_data;
    Stage_State stage_state;
    Particle_Pool particle_pool;
    Particle_Pool death_particle_pool;
    Particle_Pool stage_exit_particle_pool;
    Particle_Pool ui_particle_pool;
    // TODO: allow player name configuration
    /* char          player_name[64]; */
    void build_current_input_packet();

    void reset_for_new_run(void);

    Gameplay_Frame_Input_Packet current_input_packet;
    Gameplay_Demo_Viewer        demo_viewer;
    Gameplay_Demo_Collection_UI demo_collection_ui;
    Gameplay_Recording_File     recording;

    Fixed_Array<Bullet>                   to_create_player_bullets;
    Fixed_Array<Bullet>                   to_create_enemy_bullets;
    Fixed_Array<Enemy_Entity>             to_create_enemies;
    Fixed_Array<Pickup_Entity>            to_create_pickups;

    Fixed_Array<Particle_Emitter> ui_particle_emitters;
    Fixed_Array<Particle_Emitter> particle_emitters;
    Fixed_Array<Pickup_Entity>    pickups;
    Fixed_Array<Bullet>           bullets;
    Fixed_Array<Enemy_Entity>     enemies;
    Fixed_Array<Laser_Hazard>     laser_hazards;
    Fixed_Array<Explosion_Hazard> explosion_hazards;
    Fixed_Array<DeathExplosion>   death_explosions; // TODO(jerry):

    // NOTE: these are per frame.
    Fixed_Array<Scriptable_Render_Object> scriptable_render_objects;
    Simple_Scrollable_Background_Entities simple_scrollable_backgrounds;

    // TODO: adjust the position of these items.
    Fixed_Array<Gameplay_UI_Score_Notification> score_notifications;
    Fixed_Array<Gameplay_UI_Hitmark_Score_Notification> hit_score_notifications;
    Player              player;
    s32 player_damage_level_taken = 1;
    Cosmetic_Pet        pet;

    Boss_Healthbar_Displays boss_health_displays;
    Play_Area play_area;

    // Allow these to be disabled within lua.
    bool disable_grazing = false;
    bool disable_bullet_to_points = false;
    bool disable_enemy_to_points = false;
    bool show_damage_player_will_take = false;

    random_state prng;

    // this prng is not replicated. Used for currently only the dialogue system.
    // as it's data is not saved.
    random_state prng_unessential;

    camera       main_camera;

    bool paused_from_death = false;
    bool triggered_stage_completion_cutscene = false;
    bool queue_bomb_use = false;
    bool just_used_bomb = false;

    f32 fixed_tickrate_timer     = 0.0f;
    f32 fixed_tickrate_remainder = 0.0f;
    Gameplay_Stage_Introduction_Sequence   intro;
    Gameplay_Stage_Complete_Stage_Sequence complete_stage;

    Gameplay_Data_Particle_Spawn_Request ui_particle_spawn_queue[64];
    s32 ui_particle_spawn_queue_count=0;

    s32 selected_pet = GAME_PET_ID_NONE;
    s32 unlocked_pets = 0; // [0,3]
    s32 tries = MAX_BASE_TRIES;
    s32 max_tries = MAX_BASE_TRIES;
    s32 current_score = 0;
    f32 current_stage_timer = 0;
    u64 started_system_time = 0; // for API integrations.

    Fixed_Array<image_id> script_loaded_images;
    Fixed_Array<Audio::Sound_ID> script_loaded_sounds;

    image_id script_load_image(Game_Resources* resources, char* where);
    Audio::Sound_ID script_load_sound(Game_Resources* resources, char* where, bool streamed=false);

    void set_pet_id(s8 id, Game_Resources* resources);
    void remove_life(void);
    void add_life(void);

    void unload_all_dialogue_loaded_resources(Game_State* state, Game_Resources* resources);
    void unload_all_script_loaded_resources(Game_State* state, Game_Resources* resources);
    void process_particle_spawn_request_queue(Game_Resources* resources, s32 index, V2 current_cursor);

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

    void update_and_render_focus_mode_hitboxes(Game_State* state, struct render_commands* render_commands, Game_Resources* resources, f32 dt);

    Enemy_Entity*  lookup_enemy(u64 uid);
    Bullet* lookup_bullet(u64 uid);

    void spawn_death_explosion(V2 position);

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
    UI_STATE_CONTROLS,
    UI_STATE_STAGE_SELECT,
    UI_STATE_PET_SELECT,
    UI_STATE_DEAD_MAYBE_RETRY,
    UI_STATE_ACHIEVEMENTS,
    UI_STATE_REVIEW_SCRIPT_ERROR,
    UI_STATE_REPLAY_COLLECTION,
    UI_STATE_REPLAY_ASK_TO_SAVE,
    UI_STATE_REPLAY_NOT_SUPPORTED,
    UI_STATE_CONFIRM_BACK_TO_MAIN_MENU,
    UI_STATE_CONFIRM_EXIT_TO_WINDOWS,
    UI_STATE_SHOW_RENDERER_DISCLAIMER,
    UI_STATE_HELP_MENU,
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
#define DIALOGUE_TYPE_SPEED             (0.0327f)
#define DIALOGUE_BOX_EXPANSION_MAX_TIME (0.685f)
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

enum Dialogue_UI_Animation_Phase {
    // NOTE: characters are not updated yet.
    DIALOGUE_UI_ANIMATION_PHASE_INTRODUCTION = 0,
    DIALOGUE_UI_ANIMATION_PHASE_IDLE         = 1,
    // NOTE: the text typing is *part* of the normal update, and I don't
    // see a need to make it a special phase
    DIALOGUE_UI_ANIMATION_PHASE_UNWRITE_TEXT = 2, // TODO: figure out when this is needed.
    DIALOGUE_UI_ANIMATION_PHASE_BYE          = 3,
};

local string dialogue_ui_animation_phase_strings[] = {
    string_literal("(introduction)"),
    string_literal("(idle)"),
    string_literal("(unwrite-text)"),
    string_literal("(bye)"),
};

// NOTE(jerry):
// This hasn't been used in a real long time huh?
//
// Honestly, in the direction the game is running, I don't think it will be needed but
// it's fine to keep *in-case*?
#define DIALOGUE_MAX_LINE_LENGTH (128)
struct Dialogue_State {
    uint8_t phase = DIALOGUE_UI_ANIMATION_PHASE_INTRODUCTION;
    bool in_conversation = false;
    bool confirm_continue = false; // flag to allow the script to continue executing
    bool speaking_lines_of_dialogue = false; // flag to determine whether the text should be "rewritten"
    // Only two characters would ever really be speaking.
    Dialogue_Speaker speakers[2];
    char             current_line[DIALOGUE_MAX_LINE_LENGTH];
    char             next_line[DIALOGUE_MAX_LINE_LENGTH];

    s16 shown_characters    = 0; // for typed out text
    s16 length              = 0;  // (unused?)
    f32 type_timer          = 0.0f;
    f32 box_open_close_timer = 0.0f;

    // I don't really trust my scissor box implementation, and also using the scissor
    // box would definitely look worse than actually untyping the whole character.
    // this calculation is used so I can "untype" characters at the same rate the dialogue
    // box is closing.
    f32 bye_optimal_untype_time_max = 0.0f; // based on character count

    int      tracked_image_count = 0;
    image_id tracked_images[512];
    /* Fixed_Array<image_id>        tracked_images; */
    /* Fixed_Array<Audio::Sound_ID> tracked_sounds; */
};

enum DeathScreen_Phase {
    DEATH_SCREEN_PHASE_SLIDE_IN      = 0,
    DEATH_SCREEN_PHASE_TRY_FOR_REPLAY = 5,
    DEATH_SCREEN_PHASE_FADE_IN_TEXT  = 1,
    DEATH_SCREEN_PHASE_FADE_OUT_TEXT = 2,
    DEATH_SCREEN_PHASE_SLIDE_OUT     = 3, // allowing a replay...
    DEATH_SCREEN_PHASE_BYE           = 4,
};
/*
  NOTE:

  I originally wanted a more elaborate death animation,
  but since this is an arcadey game. This would be a terrible idea,
  and someone might get bored pretty quick if they sit through a long
  elaborate death animation.

  So I'm going to make it quick and simple...

  The main change was to make it look less placeholdery than whatever I'd
  already been using.
*/

// TODO: make like a 3 second death track.
#define MAX_DEATH_BLACK_FADE_T (0.75f)
#define MAX_DEATH_TEXT_FADE_T  (0.75f)
#define MAX_DEATH_TEXT_HOLD_T  (0.85f)
#define MAX_DEATH_TEXT_PHASE_LENGTH_T (MAX_DEATH_TEXT_FADE_T + MAX_DEATH_TEXT_HOLD_T)
struct DeathScreen_Data {
    // NOTE: this has to be drawn on top always.
    // this is a side swipe and works differently
    // from the transition system.
    s32 phase = DEATH_SCREEN_PHASE_SLIDE_IN;
    f32 black_fade_t = 0.0f;
    f32 text_fade_t  = 0.0f;
    void reset(void);
};

/*
  NOTE:
  2/1/2024,

  Better animation tooling would take much more time, and coding animations
  like this is relatively comfortable to be honest in C++ since everything
  is pretty easy to reason about.

  Although I admit I should utilize methods more to separate stuff, as a solo
  person I find it easier for me to personally to go through big functions demarked
  with something like #region.

  If there were someone else with me, I would most definitely not do this lol.
*/
enum DeathAnimation_Phase {
    DEATH_ANIMATION_PHASE_INACTIVE = 0,
    DEATH_ANIMATION_PHASE_FLASH    = 1,
    DEATH_ANIMATION_PHASE_LINGER   = 2,
};

#define DEATH_ANIMATION_MAX_T_PER_FLASH (0.15)
#define DEATH_ANIMATION_LINGER_MAX_T    (2.00f)
#define DEATH_ANIMATION_FLASH_AMOUNT    (5)
struct DeathAnimation_Data {
    // sprout 5 times outward (kinda like megaman!)
    u8               phase       = DEATH_ANIMATION_PHASE_INACTIVE;
    Particle_Emitter player_explosion_emitter;
    f32              t           = 0.0f;
    f32              flash_t     = 0.0f;
    // I admit, I'd like to do a more retro effect and invert the entire screen
    // however unlike legends-jrpg, the postprocessing pipeline is mostly gone
    // (even though there's a lot of resident code for it.)

    // The postprocessing pipeline isn't even implemented for either hardware path,
    // so this is the best I got.
    s32              flash_count = 0;
    bool             flashing    = false;
};

enum Help_Menu_Page {
  HELP_MENU_PAGE_TOC,
  HELP_MENU_PAGE_FOCUS_MODE,
  HELP_MENU_PAGE_PLAY_AREA,
  HELP_MENU_PAGE_SCORING,
  HELP_MENU_PAGE_PETS,
  HELP_MENU_PAGE_BURST_METER_GENERAL,
  HELP_MENU_PAGE_BURST_METER_LASER,
  HELP_MENU_PAGE_BURST_METER_SHIELD,
  HELP_MENU_PAGE_BURST_METER_BOMB,
  HELP_MENU_PAGE_COUNT,
};

struct Help_Menu_Data {
  int page;
};

struct Game_State {
    s32 screen_mode      = GAME_SCREEN_DEFAULT_MODE;
    s32 last_screen_mode = GAME_SCREEN_DEFAULT_MODE;
    s32 ui_state         = UI_STATE_INACTIVE;
    s32 last_ui_state    = UI_STATE_INACTIVE;
    s32 last_completion_state = -1;

    bool viewed_renderer_change_disclaimer = false;

    Controller_LED_State led_state;

    DeathAnimation_Data deathanimation_data;

    DeathScreen_Data deathscreen_data;
    Gameplay_Data    gameplay_data;
    MainMenu_Data    mainmenu_data;
    TitleScreen_Data titlescreen_data;
    Trailer_Cutaway_Mode_Data trailer_cutaway_data;
    OpeningMode_Data opening_data;
    EndingMode_Data  ending_data;
    Achievement_Menu_Data achievement_menu;

    Help_Menu_Data help_menu_data;
    Dialogue_State dialogue_state;

    Game_Task_Scheduler coroutine_tasks;
    Game_Resources* resources;

    lua_State* alloc_lua_bindings();

    void set_led_primary_color(color32u8 color);
    void set_led_target_color_anim(color32u8 color, f32 anim_length, bool overridable=true, bool fade_back_when_done=true);
    void set_led_target_color_anim_force(color32u8 color, f32 anim_length, bool overridable=true, bool fade_back_when_done=true);

    void kill_all_bullets();
    void kill_all_enemies();
    void convert_bullets_to_score_pickups(float radius=9999, float value=100);
    void convert_enemies_to_score_pickups(float radius=9999, float value=100);
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

enum Hero_Image_Frame {
    HERO_IMAGE_FRAME_IDLE0                 = 0,
    HERO_IMAGE_FRAME_FLOAT_FRONT           = 1,
    HERO_IMAGE_FRAME_FLOAT_LEFT            = 2,
    HERO_IMAGE_FRAME_FLOAT_RIGHT           = 3,
    HERO_IMAGE_FRAME_FLOAT_BACK            = 4,
    HERO_IMAGE_FRAME_FLOAT_BACK_LEAN_LEFT  = 5,
    HERO_IMAGE_FRAME_FLOAT_BACK_LEAN_RIGHT = 6,
    HERO_IMAGE_FRAME_COUNT                 = 7,
};

enum Pet_Image_Sprite_Facing_Direction {
    PET_IMAGE_SPRITE_FACING_DIRECTION_FRONT = 1,
    PET_IMAGE_SPRITE_FACING_DIRECTION_BACK  = 0,
    PET_IMAGE_SPRITE_FACING_DIRECTION_LEFT  = 2,
    PET_IMAGE_SPRITE_FACING_DIRECTION_RIGHT = 3,
    PET_IMAGE_SPRITE_FACING_DIRECTION_COUNT = 4,
};

enum Portal_Image_Frame {
    PORTAL_IMAGE_FRAME0 = 0,
    PORTAL_IMAGE_FRAME1 = 1,
    PORTAL_IMAGE_FRAME2 = 2,
    PORTAL_IMAGE_FRAME3 = 3,
    PORTAL_IMAGE_FRAME4 = 4,
    PORTAL_IMAGE_FRAME_COUNT = 5,
};

enum UI_HP_Icon_Type {
    UI_HP_ICON_TYPE_DEAD   = 0,
    UI_HP_ICON_TYPE_LIVING = 1,
    UI_HP_ICON_TYPE_CONTINUE = 2,
};

enum Game_Side_Marquee_Neo_Theme {
  GAME_SIDE_MARQUEE_NEO_THEME_BLUE = 0,
  GAME_SIDE_MARQUEE_NEO_THEME_GREEN = 1,
  GAME_SIDE_MARQUEE_NEO_THEME_RED = 2,
  GAME_SIDE_MARQUEE_NEO_THEME_COUNT = 3,
};

/*
 * NOTE: all of the "permenant" or otherwise code referenced assets are here
 *
 * This remains here since it was a shortsighted decision. It's not the *worst* thing
 * since the asset count is relatively small, and some things that are genuinely not hard-coded
 * are flexible.
 */
struct Game_Resources {
    struct graphics_assets graphics_assets;
    font_id         menu_fonts[MENU_FONT_COUNT];
    image_id        circle;
    image_id        bubbleshield_circle;
    image_id        circle16;

    // These are currently not animated because I don't have the budget to
    // animate these more thoroughly myself :P
    image_id pet_images[GAME_PET_ID_COUNT][4];
    image_id hero_images[HERO_IMAGE_FRAME_COUNT];
    image_id main_menu_portal_images[PORTAL_IMAGE_FRAME_COUNT];

    image_id main_menu_clutter_poop;
    image_id ui_hp_icons[4];

    // TODO: would like to load these from files.
    // I can't really hot reload these because of the way sprites work and my memory allocation
    // scheme which doesn't really support lots of dynamic shifting like this would require.
    sprite_id       projectile_sprites[PROJECTILE_SPRITE_TYPES];
    sprite_id       entity_sprites[ENTITY_SPRITE_TYPES];

    sprite_id       player_sprite;
    sprite_id       circle_sprite;
    sprite_id       circle_sprite16;
    sprite_id       point_pickup_sprite;

    // NOTE: the pet sprites only need to exist for the back facing sprite...
    sprite_id pet_sprites[GAME_PET_ID_COUNT];

    // Title screen assets
    // NOTE: honestly, I'd rather just animate the thing manually since it's
    // not that much code, and also less cumbersome.
    // The sprite stuff is mainly intended for the ingame screen and is not as complex
    // as a proper sprite system.
    // NOTE: this is for a "split" piece animated object for the title screen
    // kind of like Spline, but budget mode.
    image_id title_screen_puppet_eye_brow;
    image_id title_screen_puppet_eyes[4];
    image_id title_screen_puppet_head;
    image_id title_screen_puppet_arm;
    image_id title_screen_puppet_torso;
    image_id title_screen_logo_fills[3];
    image_id title_screen_logo_masks[3];

    // NOTE(jerry): these assets are not shipped with the game at all!
    // they're set up for the development repository paths.
    image_id trailer_platform_logos[TRAILER_CUTAWAY_PLATFORM_LOGO_COUNT];
    image_id trailer_storefront_logos[TRAILER_CUTAWAY_STOREFRONT_LOGO_COUNT];

    // end title screen

    image_id        ui_marquee_bkrnd;
    image_id        ui_marquee_bkrnd_neo[3][2];

    image_id        ui_vignette_borders[2];
    image_id        ui_rays_gradient;
    image_id        ui_border_effect[BORDER_FLASH_ID_TYPE_COUNT-1];
    image_id        explosion_image[3];

    image_id        ui_controller_button_icons[16]; // no input.h inclusion
    image_id        ui_burst_meter_action_icons[4]; // If I were to have more burst features, should be differently sized, but one character is already enough work!

    // Technically I'm using this for a cheap "glow" effect
    image_id        ui_border_vignette;

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

    // Contains all projectiles and entity sprites.
    // This is anything that isn't loaded in "on-demand" from scripts
    Texture_Atlas    gameplay_texture_atlas;

    // I should have more of these...
    Audio::Sound_ID        attack_sounds[4];
    Audio::Sound_ID        explosion_sounds[4];
    Audio::Sound_ID        hit_sounds[2];
    Audio::Sound_ID        opening_beep_type;
    Audio::Sound_ID        death_sound;
    Audio::Sound_ID        score_pickup_sound;

    // NOTE(jerry):
    // these tracks are looped.
    Audio::Sound_ID        intro_music;
    Audio::Sound_ID        title_music;
    Audio::Sound_ID        title_reprisal_music;

    // weird special case for bullets since those actually
    // require rotation.
    //
    // It's being used by entity rendering as a hint basically.
    bool sprite_id_should_be_rotated(sprite_id id);

    inline Audio::Sound_ID random_explosion_sound(struct random_state* prng) {
      return explosion_sounds[
        random_ranged_integer(prng, 0, array_count(explosion_sounds) - 1)
      ];
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
