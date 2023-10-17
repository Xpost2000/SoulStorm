#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "entity.h"
#include "fixed_array.h"

// this is where the actual member variables of the game go
// the class is just a small wrapper for this stuff
struct Game_State {
    Fixed_Array<Bullet> bullets;
    // should be in a separate list of it's own...
    Player              player;

    // should also have some form of entity iterator
    // or something later.
};

// No methods, just a bunch of free functions

#endif
