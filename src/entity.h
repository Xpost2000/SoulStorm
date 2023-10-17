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
    bool triggered();

    bool running = false;
    f32 t, hit_t;
    f32 max_t;
};

struct Entity {
    // primarily for collision purposes
    // a visual representation can be drawn separately
    V2 position;
    V2 scale; 
    V2 velocity;

    float t_since_spawn = 0.0f;

    // I normally don't like using these... I still don't!
    virtual void draw(software_framebuffer* framebuffer);
    // NOTE: I only want read only states
    // wholesale updates like collisions will be checked in the main
    // game loop in game.cpp
    virtual void update(Game_State* const state, f32 dt);
    
    // entities will be centered on themselves
    rectangle_f32 get_rect();
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
