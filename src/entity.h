/*
 * This is a "mega-header" which will include all the other entity
 * related header files.
 *
 * NOTE: some cruft related from when I still wanted to write a decent
 * amount of the game code native and not in lua.
 *
 */

#ifndef ENTITY_H
#define ENTITY_H

#include "render_commands.h"
#include <functional>

#define DEFAULT_SHOW_FLASH_WARNING_TIMER (0.125)
#define DEFAULT_SHOW_FLASH_WARNING_TIMES (5)
#define PLAYER_INVINICIBILITY_TIME   (1.35)
#define PLAYER_GRAZE_RADIUS          (45.0f)
// this needs to be applied in order to avoid abusing
// grazing to rapidly gain score.
#define GRAZING_SCORE_FIRE_RADIUS        (150.0f)  
#define GRAZING_SCORE_AWARD_PICKUP_DELAY (0.100f)
#define PICKUP_ENTITY_AUTO_ATTRACT_DELAY (0.15f)
#define PLAYER_DEFAULT_GRAZING_DELAY (0.075f)
#define GRAZING_SCORE_AWARD_DELAY    (0.050f)
#define GRAZING_DEFAULT_SCORE_AWARD  (100)
#define PICKUP_ENTITY_DEFAULT_LIFETIME (7.4f)

#define INVINCIBILITY_FLASH_TIME_PERIOD (PLAYER_INVINICIBILITY_TIME / 20) / 2
#define ENTITY_TIME_BEFORE_OUT_OF_BOUNDS_DELETION (5.5f)
#define DEFAULT_FIRING_COOLDOWN (0.125)
#define DEFAULT_ENTITY_SCORE_VALUE_PER_HIT (30)
#define DEFAULT_ENTITY_SCORE_KILL_VALUE_MULTIPLIER (5)
#define MAX_PREVIOUS_POSITIONS_FOR_TRAIL (64)
#define PICKUP_ENTITY_ANIMATION_T_LENGTH (0.75f)
#define PICKUP_ENTITY_FADE_T_LENGTH      (0.10f)

// This is a basic timer intended to do native timing related
// things. Although the "game-plan" is to use Lua Coroutines to do
// most of the nasty timing business.

struct Game_State; // forward decl. Entities will need to be able to look at game_state
struct Play_Area;
struct Game_Resources;

struct Timer {
    Timer();
    Timer(f32 hit);

    // While I could have a global timer service
    // I think it's just better to have entities update
    // their own timers.
    void update(f32 dt);
    void start();
    void stop();
    void reset();
    void trigger_immediate();

    // NOTE: has side effect.
    bool  triggered();
    float percentage();

    bool running = false;
    f32 t, hit_t, max_t;
};

struct Position_Trail_Ghost {
    V2 position;
    f32 alpha = 1.0f; // this will always be hardcoded to take one second
};

// Need to start removing some redundant fields or fields
// that some smaller entities don't really need.
// since I inherit from this for shared behavior regarding kinematics,
// and automatic timer tracking (t_since_spawn).
struct Entity {
    u64 uid = 0; // NOTE: this UID is for the game_state lists. This has no meaning otherwise.
    // primarily for collision purposes
    // a visual representation can be drawn separately
    V2 last_position     = V2(0, 0); // for interpolation purposes
    V2 position          = V2(0, 0);
    // NOTE: does this need interpolation?
    V2 relative_position = V2(0, 0);
    V2 scale             = V2(0, 0); 

    // NOTE: Going to take an... Alternate
    // approach to movement through scripting
    // which is more "classic".
    // It's more annoying, but it's the safest method
    V2    velocity      = V2(0, 0);
    V2    acceleration  = V2(0, 0);

    float maximum_speed = 99999999;

    f32   t_since_spawn = 0.0f;
    Sprite_Instance sprite;

    // Cool visual effect for trailing.
    color32f32           trail_ghost_modulation = color32f32(0.9, 0.9, 0.9, 1.0);
    Position_Trail_Ghost trail_ghosts[MAX_PREVIOUS_POSITIONS_FOR_TRAIL];
    s16                  trail_ghost_count = 0;
    s16                  trail_ghost_limit = 0;
    f32                  trail_ghost_record_timer = 0, trail_ghost_record_timer_max = 0.033f;
    f32                  trail_ghost_max_alpha = 0.23f;

    Timer cleanup_time                    = Timer(ENTITY_TIME_BEFORE_OUT_OF_BOUNDS_DELETION);
    Timer invincibility_time_flash_period = Timer(INVINCIBILITY_FLASH_TIME_PERIOD);
    Timer invincibility_time              = Timer(PLAYER_INVINICIBILITY_TIME);

    // TODO: enforce this later.
    //       After checkpoint 1 when I clean most of this up
    s32   hp                              = 1;
    s32   max_hp                          = 1;

    f32  hp_percentage();

    // NOTE: this should become a bit field in the future.
    //like... now.
    bool  die                             = false; // force dead flag

    // TODO: allow flashing color adjustment
    bool  flashing                        = false; // flicker on or off
    bool  invincibility_show_flashing     = true;
    bool  allow_out_of_bounds_survival    = false;

    // if timer == -1, be invincible forever.
    void end_invincibility();
    void begin_invincibility(bool visual_flash=true, f32 timer=PLAYER_INVINICIBILITY_TIME);

    f32   firing_t      = 0;
    f32   firing_cooldown = DEFAULT_FIRING_COOLDOWN;
    bool  firing        = false;

    s32   score_value      = DEFAULT_ENTITY_SCORE_VALUE_PER_HIT;
    s16   death_multiplier = DEFAULT_ENTITY_SCORE_KILL_VALUE_MULTIPLIER;

    bool attack();
    void stop_attack();

    // I normally don't like using these... I still don't!
    virtual void draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources);
    // NOTE: I only want read only states
    // wholesale updates like collisions will be checked in the main
    // game loop in game.cpp
    virtual void update(Game_State* state, f32 dt);

    // entities will be centered on themselves
    rectangle_f32 get_rect();

    // but that's more of a polish thing
    // damage could spawn a particle or something
    bool damage(s32 dmg); // NOTE: player should have a bit of invulnerability after respawning.
    // TODO: need to track hp max, but this is mostly for the player
    //       who should basically die in one hit as per normal bullet hell rules...
    bool heal(s32 hp);
    bool kill();

    // -1 means we'll follow the stage area
    s8 edge_top_behavior_override = -1;
    s8 edge_bottom_behavior_override = -1;
    s8 edge_left_behavior_override = -1;
    s8 edge_right_behavior_override = -1;

protected:
    bool touching_left_border(const Play_Area& play_area, bool as_point=false);
    bool touching_right_border(const Play_Area& play_area, bool as_point=false);
    bool touching_top_border(const Play_Area& play_area, bool as_point=false);
    bool touching_bottom_border(const Play_Area& play_area, bool as_point=false);

    bool clamp_to_left_border(const Play_Area& play_area);
    bool clamp_to_right_border(const Play_Area& play_area);
    bool clamp_to_top_border(const Play_Area& play_area);
    bool clamp_to_bottom_border(const Play_Area& play_area);

    bool wrap_from_left_border(const Play_Area& play_area);
    bool wrap_from_right_border(const Play_Area& play_area);
    bool wrap_from_top_border(const Play_Area& play_area);
    bool wrap_from_bottom_border(const Play_Area& play_area);

    bool wrap_border(s32 edge, const Play_Area& play_area);
    bool clamp_border(s32 edge, const Play_Area& play_area);
    bool touching_border(s32 edge, const Play_Area& play_area, bool as_point=false);

    void handle_play_area_edge_behavior(const Play_Area& play_area);

private:
    inline void update_ghost_trails(f32 dt);
    inline void handle_out_of_bounds_behavior(const Play_Area& play_area, f32 dt);
    inline void handle_invincibility_behavior(f32 dt);
    inline void update_firing_behavior(f32 dt);
};

struct Enemy_Entity;
using Enemy_Entity_Fire_Fn = std::function<void(Enemy_Entity*, Game_State*, f32)>;
using Enemy_Entity_Velocity_Fn = std::function<void(Enemy_Entity*, Game_State* const, f32)>;

struct Enemy_Entity : public Entity {
    Timer outside_boundaries_lifetime_timer = Timer(10.0f);
    Enemy_Entity_Velocity_Fn velocity_function = nullptr;
    Enemy_Entity_Fire_Fn     on_fire_function  = nullptr;

    void update(Game_State* state, f32 dt);

    // NOTE: I would like to multithread the bullets and particles and entity movements
    //       however for specifically entities that fire things, this is not possible
    //       to multithread because Fixed_Array is not thread_safe (and also it would defeat the purpose of threading in this architecture.)
    void try_and_fire(Game_State* state, f32 dt);
    void reset_movement();
};

struct Hazard_Warning {
    Hazard_Warning(f32 amount_of_time_for_warning);
    Hazard_Warning();

    Timer warning_flash_timer;
    Timer show_flash_timer;
    s32   flash_warning_times;
    bool  presenting_flash;

    void update(f32 dt);
    bool finished_presenting();
};

/* Hazard types */
//
// While technically "entities" per say, they're not
// really meant to be big objects like regular entities,
// so I'm keeping these completely separate.
// NOTE: will need to spawn a particle system when it dies

// These are still designed to be relatively "parallel" friendly
// so I'll spawn the hazards these things are supposed to be spawning inside of
// the game loop, instead of as part of the method.
// It simplifies the logic in my opinion by just keeping these in the main game code.

// NOTE: position is centered.
struct Explosion_Hazard {
    Explosion_Hazard(V2 position, f32 radius, f32 amount_of_time_for_warning, f32 time_until_explosion);
    Explosion_Hazard();

    V2  position;
    f32 radius;

    Hazard_Warning warning;
    bool on_presenting_flash_events;

    Timer explosion_timer;

    bool exploded = false;

    void update(Game_State* state, f32 dt);
    void draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources);
};

enum Laser_Hazard_Direction {
    LASER_HAZARD_DIRECTION_HORIZONTAL,
    LASER_HAZARD_DIRECTION_VERTICAL,
};

struct Laser_Hazard {
    Laser_Hazard(float position, float radius, int direction, float amount_of_time_for_warning, float how_long_to_live);
    Laser_Hazard();

    float radius    = 0.0f;
    float position  = 0.0f;
    // NOTE: it's probably really hard to design for this
    // since lasers are really "area boundary" controllers
    // and moving them is really complicated...
    // but it might be okay since there's not many lasers so I can probably
    // just use a lua script for these
    float velocity  = 0.0f;
    int   direction = LASER_HAZARD_DIRECTION_HORIZONTAL;

    Hazard_Warning warning;
    bool on_presenting_flash_events;

    Timer lifetime;
    bool  die = false; // force kill flag
    bool  already_emitted = false;

    void update(Game_State* state, f32 dt);
    void draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources);
    bool ready();

    // since it relies on the size of the play area
    rectangle_f32 get_rect(const Play_Area* area);
};

// NOTE: for slightly different player types, I might need to rework some player behaviors...
//       This was not totally planned for, but I might just keep only one player type so I can actually
//       meet my arbitrary deadline.
struct Player : public Entity {
    // I did have plans for slightly different player types to make it interesting
    // but I'll see about that later.
    void update(Game_State* state, f32 dt);
    s32  currently_grazing(Game_State* state);
    void handle_grazing_behavior(Game_State* state, f32 dt);
    f32  get_grazing_score_modifier(s32 amount);

    f32 grazing_award_timer = 0.0f;
    f32 grazing_award_score_pickup_timer = 0.0f;
    f32 grazing_delay       = PLAYER_DEFAULT_GRAZING_DELAY;
    f32 time_spent_grazing  = 0.0f;

    // a focused character will be slower and shoot 'harder' and faster.
    bool under_focus = false;
};

enum Bullet_Source {
    // Maybe I just want to shoot random bullets in the environment to kill stuff.
    // who knows?
    BULLET_SOURCE_NEUTRAL, 

    BULLET_SOURCE_PLAYER,

    BULLET_SOURCE_ENEMY,
};

struct Bullet;
using Bullet_Entity_Velocity_Fn = std::function<void(Bullet*, Game_State* const, f32)>; 
struct Bullet : public Entity {
    Timer lifetime; // should be adjusted carefully!

    /*
      NOTE:
      There's never a reason to explicitly know which exact
      entity shot the bullet.
     */
    s32   source_type;

    void update(Game_State* state, f32 dt);
    void reset_movement();

    // want this to be handled via lua
    Bullet_Entity_Velocity_Fn velocity_function = nullptr;
private:
    void handle_lifetime(f32 dt);
    void handle_movement(Game_State* state, f32 dt);
};

// TODO: score entities need lua bindings!
enum Pickup_Entity_Type {
    PICKUP_SCORE,
    PICKUP_ATTACKPOWER, // NOTE: not sure if I want this.
    // NOTE: a direct life pickup should be relatively uncommon.
    PICKUP_LIFE, 
};

/*
 * These entities will always animate to their final position before
 * settling down.
 */
struct Pickup_Entity : public Entity {
    s32 type = PICKUP_SCORE;

    // TODO: custom drawing override.
    //       for the slightly different fading!
    void chase_player_update(Game_State* state, f32 dt);
    void default_behavior_update(Game_State* state, f32 dt);
    void update(Game_State* state, f32 dt);
    void on_picked_up(Game_State* state);

    Timer lifetime = Timer(PICKUP_ENTITY_DEFAULT_LIFETIME);

    // NOTE:
    // this is "purely visual"
    // and is an overriding behavior.
    // a pickup entity that seeks towards the player
    // will ignore every other flag here because they
    // don't have to animate, and effectively are "guaranteed points"
    bool seek_towards_player = false;

    bool awarded             = false;
    f32  fading_t            = 0.0f;
    s32  fade_phase          = 0; // even is visible, odd is invisible.
    s32  value               = 0;


    // NOTE: timing is hard coded, and non-adjustable.
    f32 animation_t = 0.0f;
    V2  animation_start_position;
    V2  animation_end_position;
};

Pickup_Entity pickup_score_entity(Game_State* state,        V2 start, V2 end, s32 value);
Pickup_Entity pickup_attack_power_entity(Game_State* state, V2 start, V2 end, s32 value);
Pickup_Entity pickup_life_entity(Game_State* state,         V2 start, V2 end);

struct lua_State;
void bind_entity_lualib(lua_State* L);

#include "entity_prototypes.h"

#endif
