#ifndef ENTITY_H
#define ENTITY_H

#include "graphics.h"
#include "common.h"
#include "v2.h"
/*
 * all entity type definitions here
 * for now, everyone is going to be specialized until I wanna do something else.
 */

/*
  10/16/2023
  NOTE: I want to experiment with building patterns through basic manipulation of
        velocity
 */
struct Entity {
    // primarily for collision purposes
    // a visual representation can be drawn separately
    // (NOTE: not ECS style attached since that's a bit weird for now)
    V2 position;
    V2 scale; 
    V2 velocity;

    // I normally don't like using these... I still don't!
    virtual void draw(software_framebuffer* framebuffer);
    virtual void update(f32 dt);
    
    // entities will be centered on themselves
    rectangle_f32 get_rect();
};

struct Player : public Entity {
    // I think I just want the instant death system, which is a bit painful but
    // okay! Just have a bunch of lives.

    // I did have plans for slightly different player types to make it interesting
    // but I'll see about that later.
    void update(f32 dt);
};
struct Bullet : public Entity {
    float lifetime; // if it's -1 the bullets will die on their own later...
    bool dead;
    void update(f32 dt);
};

#endif
