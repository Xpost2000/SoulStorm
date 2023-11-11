#ifndef ENTITY_H
#define ENTITY_H

// should vary by "difficulty"
#define DEFAULT_SHOW_FLASH_WARNING_TIMER (0.125)
#define DEFAULT_SHOW_FLASH_WARNING_TIMES (5)

#include "render_commands.h"

#include <functional>
/*
 * This is a "mega-header" which will include all the other entity
 * related header files.
 */

// This is a basic timer intended to do native timing related
// things. Although the "game-plan" is to use Lua Coroutines to do
// most of the nasty timing business.

// I also don't want to implement another lisp interpreter...
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

#define PLAYER_INVINICIBILITY_TIME (1.35)
#define INVINCIBILITY_FLASH_TIME_PERIOD (PLAYER_INVINICIBILITY_TIME / 20) / 2
#define ENTITY_TIME_BEFORE_OUT_OF_BOUNDS_DELETION (5.0f)
#define DEFAULT_FIRING_COOLDOWN (0.125)
#define DEFAULT_ENTITY_SCORE_VALUE_PER_HIT (30)
#define DEFAULT_ENTITY_SCORE_KILL_VALUE_MULTIPLIER (5)
#define MAX_PREVIOUS_POSITIONS_FOR_TRAIL (32)
struct Position_Trail_Ghost {
    V2 position;
    f32 alpha = 1.0f; // this will always be hardcoded to take one second
};

// NOTE: no real system for a visual sprite quite yet.
struct Entity {
    u64 uid;
    // primarily for collision purposes
    // a visual representation can be drawn separately
    V2    position      = V2(0, 0);
    V2    scale         = V2(0, 0); 

    // NOTE: Going to take an... Alternate
    // approach to movement through scripting
    // which is more "classic".
    // It's more annoying, but it's the safest method
    V2    velocity      = V2(0, 0);
    V2    acceleration  = V2(0, 0);

    f32   t_since_spawn = 0.0f;
    Sprite_Instance sprite;

    // Cool visual effect for trailing.
    color32f32           trail_ghost_modulation = color32f32(0.9, 0.9, 0.9, 1.0);
    Position_Trail_Ghost trail_ghosts[MAX_PREVIOUS_POSITIONS_FOR_TRAIL];
    s32                  trail_ghost_count = 0;
    s32                  trail_ghost_limit = 0;
    f32                  trail_ghost_record_timer = 0, trail_ghost_record_timer_max = 0.033f;
    f32                  trail_ghost_max_alpha = 0.23f;

    Timer cleanup_time                    = Timer(ENTITY_TIME_BEFORE_OUT_OF_BOUNDS_DELETION);
    Timer invincibility_time_flash_period = Timer(INVINCIBILITY_FLASH_TIME_PERIOD);
    Timer invincibility_time              = Timer(PLAYER_INVINICIBILITY_TIME);
    int   hp            = 1;
    bool  die           = false; // force dead flag
    bool  flashing      = false; // flicker on or off

    f32   firing_t      = 0;
    f32   firing_cooldown = DEFAULT_FIRING_COOLDOWN;
    bool  firing        = false;

    s32   score_value      = DEFAULT_ENTITY_SCORE_VALUE_PER_HIT;
    s32   death_multiplier = DEFAULT_ENTITY_SCORE_KILL_VALUE_MULTIPLIER;

    bool attack();

    // I normally don't like using these... I still don't!
    virtual void draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources);
    // NOTE: I only want read only states
    // wholesale updates like collisions will be checked in the main
    // game loop in game.cpp
    virtual void update(Game_State* state, f32 dt);

    // entities will be centered on themselves
    rectangle_f32 get_rect();

    /*
      These are helpers that are used to allow for the play area border to
      do certain things.
     */

    // If I need some more specific behaviors...
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

    void handle_play_area_edge_behavior(const Play_Area& play_area);

    // but that's more of a polish thing
    // damage could spawn a particle or something
    bool damage(s32 dmg); // NOTE: player should have a bit of invulnerability after respawning.
    // TODO: need to track hp max, but this is mostly for the player
    //       who should basically die in one hit as per normal bullet hell rules...
    bool heal(s32 hp);
    bool kill();

    // -1 means we'll follow the stage area
    s32 edge_top_behavior_override = -1;
    s32 edge_bottom_behavior_override = -1;
    s32 edge_left_behavior_override = -1;
    s32 edge_right_behavior_override = -1;
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

    bool exploded;

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
    // I think I just want the instant death system, which is a bit painful but
    // okay! Just have a bunch of lives.

    // I did have plans for slightly different player types to make it interesting
    // but I'll see about that later.
    void update(Game_State* state, f32 dt);

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

    // want this to be handled via lua
    Bullet_Entity_Velocity_Fn velocity_function = nullptr;
};

#include "entity_prototypes.h"

#endif
