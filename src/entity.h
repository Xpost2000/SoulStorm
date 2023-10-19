#ifndef ENTITY_H
#define ENTITY_H

#include "graphics.h"
#include "common.h"
#include "v2.h"

#include <functional>
/*
 * all entity type definitions or entity related code in here
 * it's a small game so this is okay.
 *
 * for now, everyone is going to be specialized until I wanna do something else.
 * Starting is always so difficult to do architecture for.
 */

/*
  10/16/2023
  NOTE: I want to experiment with building patterns through basic manipulation of
        velocity. I think I can get a lot of milage out of just doing this alone so
        I hope my idea pans out.

        IE: clever usage of waits + velocity functions should make things look cool.
 */

// This is a basic timer intended to do native timing related
// things. Although the "game-plan" is to use Lua Coroutines to do
// most of the nasty timing business.

// I also don't want to implement another lisp interpreter...
struct Game_State; // forward decl. Entities will need to be able to look at game_state
struct Game_Resources;

struct Timer {
    Timer(f32 hit);

    // While I could have a global timer service
    // I think it's just better to have entities update
    // their own timers.
    void update(f32 dt);
    void start();
    void stop();
    void reset();

    // NOTE: has side effect.
    bool  triggered();
    float percentage();

    bool running = false;
    f32 t, hit_t;
    f32 max_t;
};

struct Entity {
    // primarily for collision purposes
    // a visual representation can be drawn separately
    V2    position      = V2(0, 0);
    V2    scale         = V2(0, 0); 
    V2    velocity      = V2(0, 0);
    f32   t_since_spawn = 0.0f;

    // I normally don't like using these... I still don't!
    virtual void draw(software_framebuffer* framebuffer, Game_Resources* resources);
    // NOTE: I only want read only states
    // wholesale updates like collisions will be checked in the main
    // game loop in game.cpp
    virtual void update(Game_State* const state, f32 dt);
    
    // entities will be centered on themselves
    rectangle_f32 get_rect();
};

/* Hazard types */
//
// While technically "entities" per say, they're not
// really meant to be big objects like regular entities,
// so I'm keeping these completely separate.
// NOTE: will need to spawn a particle system when it dies
// 
struct Explosion_Hazard {
    Explosion_Hazard(V2 position, f32 radius, f32 amount_of_time_for_warning, f32 time_until_explosion);
    Explosion_Hazard();

    V2  position;
    f32 radius;

    Timer warning_flash_timer;
    Timer explosion_timer;
    s32 flash_warning_times;

    void update(Game_State* const state, f32 dt);
    void draw(software_framebuffer* framebuffer, Game_Resources* resources);
};

enum Laser_Hazard_Direction {
    LASER_HAZARD_DIRECTION_HORIZONTAL,
    LASER_HAZARD_DIRECTION_VERTICAL,
};
struct Laser_Hazard {
    Laser_Hazard(float position, int direction, float amount_of_time_for_warning, float how_long_to_live);
    Laser_Hazard();

    float position  = 0.0f;
    int   direction = LASER_HAZARD_DIRECTION_HORIZONTAL;
    Timer warning_flash_timer;
    Timer lifetime;

    void update(Game_State* const state, f32 dt);
    void draw(software_framebuffer* framebuffer, Game_Resources* resources);
};

struct Player : public Entity {
    // I think I just want the instant death system, which is a bit painful but
    // okay! Just have a bunch of lives.

    // I did have plans for slightly different player types to make it interesting
    // but I'll see about that later.
    void update(Game_State* const state, f32 dt);
};

// NOTE: for now I'm testing out bullet patterns with
//       lambda closures because I don't have time to implement
//       lua support tonight and I wanna eat dinner soon lol.
struct Bullet : public Entity {
    /* float lifetime; // if it's -1 the bullets will die on their own later... */
    /* bool dead; */
    void update(Game_State* const state, f32 dt);

    std::function<void(Bullet*, Game_State* const, f32)> velocity_function;
};

#endif
