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
// NOTE: none of these guys fire anything.
// also I'm aware that this might be an allocation festival because I'm building up
// stuff by copying it over and over again, however it does lead to nice compositional behavior...
// in lua it should generally still work with a similar API.
#if 0
enemy = new_enemy_linear_movement(...);
enemy = new_enemy_shoots_pattern_x(enemy);
add_enemy(enemy);
#endif

Enemy_Entity enemy_linear_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed);
Enemy_Entity enemy_circular_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed, f32 r);
Enemy_Entity enemy_sine_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed, f32 amp);
Enemy_Entity enemy_cosine_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed, f32 amp);
Enemy_Entity enemy_generic(Game_State* state, V2 position, V2 scale, Enemy_Entity_Velocity_Fn velocity);

// need to rethink how entities fire stuff.

/*
  Generally, each level **should** have custom
  wave setups, but it's not impossible to reuse enemy waves sometimes
  as small filler...
 */

#endif
