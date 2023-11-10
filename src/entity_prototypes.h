#ifndef ENTITY_PROTOTYPES_H
#define ENTITY_PROTOTYPES_H

struct Game_State;
/*
  NOTE: meant to be included within entity.h together,
  this is only really split out to make it easier to
  look for individually.

  This defines a bunch of construction functions to produce
  specific types of entities that the game will use.

  Mostly for all the hard-coded C++ based entities.

  NOTE: for things like Bosses or anything with substantial
  code, they'll probably be implemented in separate files with their
  construction files in here.

  NOTE:
     these are for enemy archetypes, but specific sprites and stuff requires more work,
     and might utilize more function pointers and such.
*/


// Bullet Types
// not necessarily with sprites yet. Those need to be done separately.
Bullet bullet_upwards_linear(Game_State* state, V2 position, V2 direction, f32 magnitude, s32 source = BULLET_SOURCE_NEUTRAL);
Bullet bullet_generic(Game_State* state, V2 position, V2 scale, s32 source, Bullet_Entity_Velocity_Fn velocity);

// Bullet Patterns


// Normal Enemy Types
Enemy_Entity enemy_linear_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed);
Enemy_Entity enemy_generic(Game_State* state, V2 position, V2 scale, f32 fire_cooldown, Enemy_Entity_Velocity_Fn velocity, Enemy_Entity_Fire_Fn fire);

// Enemy Wave Presets
/*
  Generally, each level **should** have custom
  wave setups, but it's not impossible to reuse enemy waves sometimes
  as small filler...
 */

#endif
