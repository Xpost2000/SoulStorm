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

  NOTE:
     these are for enemy archetypes, but specific sprites and stuff requires more work,
     and might utilize more function pointers and such.
*/

V2 velocity_linear(V2 direction, f32 magnitude); 
V2 velocity_linear_with_accel(V2 direction, f32 magnitude, f32 t, f32 accel_magnitude);
V2 velocity_linear_with_accel(V2 direction, f32 magnitude, f32 t, V2 accel_direction, f32 accel_magnitude); 
V2 velocity_follow(Entity* entity); 
V2 velocity_circle_orbit(f32 t, f32 spawn_theta, f32 r);
V2 velocity_cyclic_sine(f32 t, V2 direction); 
V2 velocity_cyclic_cosine(f32 t, V2 direction);


// Bullet Types
// not necessarily with sprites yet. Those need to be done separately.
Bullet bullet_upwards_linear(Game_State* state, V2 position, V2 direction, f32 magnitude, s32 visual, s32 source = BULLET_SOURCE_NEUTRAL);
Bullet bullet_generic(Game_State* state, V2 position, V2 scale, s32 source, Bullet_Entity_Velocity_Fn velocity, s32 visual);

// Basic Bullet Patterns, bullet sprites are not accounted for here, but I don't have sprites yet. Hopefully soon.
/*
 * Because of the way I'd like to design this project, I did not want a very complicated way to get generic
 * bullets, and since I'm using lua, and would like to avoid making more languages I'm going to have a prefixed set of
 * bullet pattern types in native code like this.
 *
 * This means it is possible while scripting and deciding I need more ways for bullets to move I'd have to make
 * a new set of bindings, which is annoying, but it is faster than making a smaller sub language for the bullets to run scripts
 * on (I preallocate all my bullets, and lua coroutines might not be super smart on a decent amount of bullets...)
 */
void spawn_bullet_line_pattern1(Game_State* state, V2 center, s32 how_many, f32 spacing, V2 scale, V2 direction, f32 speed, s32 source, s32 visual);

// NOTE: regarding angles, they are relative to the original direction.
// NOTE: spreads out in an arc!
void spawn_bullet_arc_pattern1(Game_State* state, V2 center, s32 how_many, s32 arc_degrees, V2 scale, V2 direction, f32 speed, s32 source, s32 visual);
// NOTE: spawns and spreads from an arc!
void spawn_bullet_arc_pattern2(Game_State* state, V2 center, s32 how_many, s32 arc_degrees, V2 scale, V2 direction, f32 speed, f32 distance_from_center, s32 source, s32 visual);
void spawn_bullet_arc_pattern2_trailed(Game_State* state, V2 center, s32 how_many, s32 arc_degrees, V2 scale, V2 direction, f32 speed, f32 distance_from_center, s32 source, s32 visual, s32 trailcount);


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

/*
  The above, are still left in as simple prototypes, but this
  is preferred now, since tasks are more logical for game logic "pun-intended?".
*/
Enemy_Entity enemy_generic_with_task(Game_State* state, V2 position, V2 scale, jdr_duffcoroutine_fn task);
Bullet       bullet_generic_with_task(Game_State* state, V2 position, V2 scale, s32 source, jdr_duffcoroutine_fn task, s32 visual);

// need to rethink how entities fire stuff.

/*
  Generally, each level **should** have custom
  wave setups, but it's not impossible to reuse enemy waves sometimes
  as small filler...
 */

#endif
