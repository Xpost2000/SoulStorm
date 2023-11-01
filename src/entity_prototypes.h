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

void spawn_bullet_upwards_linear(Game_State* state, V2 position, V2 direction, f32 magnitude, s32 source = BULLET_SOURCE_NEUTRAL);
void spawn_bullet_circling_down_homing(Game_State* state, V2 position, f32 factor, f32 factor2, s32 source = BULLET_SOURCE_NEUTRAL, V2 additional = V2(0,0));
void spawn_bullet_circling_down_homing2(Game_State* state, V2 position, f32 factor, f32 factor2, s32 source = BULLET_SOURCE_NEUTRAL, V2 additional = V2(0,0));
void spawn_bullet_circling_down(Game_State* state, V2 position, f32 factor, f32 factor2, V2 additional = V2(0,0));

void spawn_enemy_linear_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed);

#endif
