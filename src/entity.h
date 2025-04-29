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
#include "particle_system.h"
#include <functional>

#include "game_rules.h"

// This is a basic timer intended to do native timing related
// things. Although the "game-plan" is to use Lua Coroutines to do
// most of the nasty timing business.

s32 calculate_amount_of_burst_depletion_flashes_for(f32 seconds);

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

#define ENTITY_MAX_PARTICLE_EMITTERS (4)

#define MINIMUM_MAGNITUDE_TO_CONSIDER_LEANING                    (0.360f)
#define MINIMUM_ADDITIONAL_MAGNITUDE_TO_CONSIDER_ROTATION_LEANING (0.12f)
#define DEFAULT_DECAY_FACTOR_MIN                                 (0.020f)
#define DEFAULT_DECAY_FACTOR_MAX                                 (0.170f)

// NOTE: I wish C++ had C99 designated initializers...
// I'll use the builder pattern to copy it for now...
struct Entity_Rotation_Lean_Params {
    Entity_Rotation_Lean_Params(f32 max_angle_lean) :
        max_angle_lean(max_angle_lean)
    {
    }

    f32 minimum_magnitude_to_consider_leaning                     = MINIMUM_MAGNITUDE_TO_CONSIDER_LEANING;
    f32 minimum_additional_magnitude_to_consider_rotation_leaning = MINIMUM_ADDITIONAL_MAGNITUDE_TO_CONSIDER_ROTATION_LEANING;
    f32 max_angle_lean                                            = 0.0f;
    f32 decay_factor_min                                          = DEFAULT_DECAY_FACTOR_MIN;
    f32 decay_factor_max                                          = DEFAULT_DECAY_FACTOR_MAX;

    Entity_Rotation_Lean_Params& update_minimum_magnitude_to_lean(f32 x) {
        this->minimum_magnitude_to_consider_leaning = x;
        return *this;
    }

    Entity_Rotation_Lean_Params& update_minimum_additional_magnitude_to_lean(f32 x) {
        this->minimum_additional_magnitude_to_consider_rotation_leaning = x;
        return *this;
    }

    Entity_Rotation_Lean_Params& update_max_angle_lean(f32 x) {
        this->max_angle_lean = x;
        return *this;
    }

    Entity_Rotation_Lean_Params& update_decay_factor_min(f32 x) {
        this->decay_factor_min = x;
        return *this;
    }

    Entity_Rotation_Lean_Params& update_decay_factor_max(f32 x) {
        this->decay_factor_max = x;
        return *this;
    }
};
enum Rotation_Lean_Influence_Direction {
    ROTATION_LEAN_INFLUENCE_DIRECTION_NOT_ROTATING,
    ROTATION_LEAN_INFLUENCE_DIRECTION_LEFT,
    ROTATION_LEAN_INFLUENCE_DIRECTION_RIGHT,
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

    s32 entity_draw_blend_mode = BLEND_MODE_ALPHA;
    s32 shadow_entity_draw_blend_mode = BLEND_MODE_ALPHA;

    V2 get_real_position(void);
    void set_position(V2 new_position);
    void set_position_x(f32 new_x);
    void set_position_y(f32 new_y);

    // NOTE: Going to take an... Alternate
    // approach to movement through scripting
    // which is more "classic".
    // It's more annoying, but it's the safest method
    V2    velocity      = V2(0, 0);
    V2    acceleration  = V2(0, 0);

    float maximum_speed = 99999999;

    f32   t_since_spawn = 0.0f;
    Sprite_Instance sprite;

    // Entity code can decide what they want to do with this one.
    Particle_Emitter emitters[ENTITY_MAX_PARTICLE_EMITTERS];

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
    bool  visible                         = true;  // this flag is used precisely (once!)
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
    
    s32   burst_pickup_value = 0;

    s32 hit_flash_count = 0;
    Timer hit_flash_timer = Timer(DAMAGE_FLASH_TIME_PERIOD);

    bool attack();
    void stop_attack();
    void post_init_apply_current_game_rules(void);

    // I normally don't like using these... I still don't!
    virtual void reset_movement(void);
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

    // Due to the intentionally very simple animation system
    // the game requires, we have to very manually control the way
    // animations are done.

    // Complex animations (sprite ones at least) aren't really a thing for
    // this engine, so I don't need much more than this...
    s32 sprite_frame_begin = -1;
    s32 sprite_frame_end   = -1;

    void disable_all_particle_emitters(void);
protected:
    void set_sprite_frame_region(s32 a=-1, s32 b=-1);
    void reset_sprite_animation_frames(void);
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

    s32 handle_play_area_edge_behavior(const Play_Area& play_area);

    // returns the leaned direction.
    s32  update_sprite_leaning_influence(f32 dt, V2 axes, Entity_Rotation_Lean_Params params);
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
// TODO: hazard types should have color settings cause bullet hells love flashiness!
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

    Particle_Emitter outer_ring_emitter;
    Particle_Emitter inner_emitter;
    Particle_Emitter dust_emitter;

    V2  position;
    f32 radius;

    color32f32 outer_ring_color = color32f32(222.0f/255.0f, 180.0f/255.0f, 45.0f/255.0f, 1.0f);
    color32f32 inner_ring_color = color32f32(108/255.0f, 122/255.0f, 137/255.0f, 1.0f);

    Hazard_Warning warning;
    Timer explosion_timer;

    bool exploded = false;
    bool on_presenting_flash_events;

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

    // NOTE: should be a projectile_sprite_id
    s32 projectile_sprite_id = -1;

    Particle_Emitter outer_ring_emitter;
    Particle_Emitter dust_emitter;

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
int get_burst_mode_rank_count(void);

struct Player : public Entity {
    // I did have plans for slightly different player types to make it interesting
    // but I'll see about that later.
    void draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources);
    void fire_weapon(Game_State* state, u32 attack_pattern_id);
    void fire_burst_ray_laser(Game_State* state);
    void handle_bomb_usage(Game_State* state, u32 bomb_pattern_id);
    void update(Game_State* state, f32 dt);
    s32  currently_grazing(Game_State* state);
    void handle_grazing_behavior(Game_State* state, f32 dt);
    void handle_burst_charging_behavior(Game_State* state, f32 dt);
    f32  get_grazing_score_modifier(s32 amount);
    s32  get_burst_rank(void);
    f32  get_burst_charge_percent(void);

    void reset_burst_charge_status(void);
    void disable_burst_charge_regeneration(void);
    void enable_burst_charge_regeneration(void);
    void halt_burst_abilities(void);
    void halt_burst_charge_regeneration(s32 flash_count_required); // flashes are at fixed times, so this
                                                                   // allows more precise gaging of time.

    void show_border_status_when_close(Game_State* state);

    f32 grazing_award_timer = 0.0f;
    f32 grazing_award_score_pickup_timer = 0.0f;
    f32 grazing_delay       = PLAYER_DEFAULT_GRAZING_DELAY;
    f32 time_spent_grazing  = 0.0f;
    f32 burst_charge = 0.0f;
    bool just_burned_out = false;

    // NOTE(jerry):
    // while I appreciate "timing" based things, it doesn't
    // seem right to do that, so I'll just do it through explicit
    // flash counts.
    bool burst_charge_disabled          = false; // should really be bit flags, but I'm a bit too lazy to deal with this.
    bool burst_charge_halt_regeneration = false;
    s32  burst_charge_flash_count  = 0;
    f32  burst_charge_halt_flash_t = 0.0f;

    // This timer is... technically updated separately from the other one
    // since this one is easier to use for viewing in a meter.
    f32  burst_charge_recharge_max_t = 0.0f;
    f32  burst_charge_recharge_t = 0.0f;

    f32  burst_absorption_shield_ability_timer = 0.0f;
    f32  burst_ray_attack_ability_timer = 0.0f;
    f32  current_burst_ability_max_t = 0.0f;

    s32  get_burst_ability_usage(s32 id);
    void add_burst_ability_usage(s32 id);

    s32  last_used_tier = -1;
    s32  burst_ability_streak_usage = 0;

    f32 drain_speed = 0.0f;

    // TODO(jerry):
    // total burst charge drainage causes delayed recharging (unless it's a bomb)
    // and disabling of all abilities until reaching tier 2 again...

    // a focused character will be slower and shoot 'harder' and faster.
    bool under_focus = false;
};

struct Cosmetic_Pet : public Entity {
    void update(Game_State* state, f32 dt);
    void set_id(s32 id, Game_Resources* resources);

    void fire_weapon(Game_State* state);
private:
    s32 id;
};

struct DeathExplosion {
  void update(Game_State* state, f32 dt);
  void draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources);
  bool dead(void);

  V2 position;
  s32 frame_count;
  s32 frame_index;
  f32 timer;
  f32 scale = 1.0f;
};

enum Bullet_Source {
    // Maybe I just want to shoot random bullets in the environment to kill stuff.
    // who knows?
    BULLET_SOURCE_NEUTRAL, 
    BULLET_SOURCE_PLAYER,
    BULLET_SOURCE_ENEMY,
    BULLET_SOURCE_COUNT,
};

static string bullet_source_strings[] = {
    string_literal("(bullet-neutral)"),
    string_literal("(bullet-player)"),
    string_literal("(bullet-enemy)"),
    string_literal("(count)"),
};

enum Bullet_Flags {
    BULLET_FLAGS_NONE                 = 0,
    BULLET_FLAGS_BREAKS_OTHER_BULLETS = BIT(0),
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
    u8    flags;

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

    void draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources);
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

/*
 * This is a native implementation for
 * Generic_Infinite_Stage_ScrollV as it's per-frame update is
 * not very performant.
 * It's written to mainly try and be simd-friendly just as a way
 * to get some easy data oriented programming style ideas.
 *
 * NOTE(jerry): these guys do not get halted for any reason for now.
 *
 * Backgrounds aren't really meant to be removed or faded/transitioned currently, if that functionality
 * is needed I guess I'll modify this...
 *
 * In any case, it's not really a lot of code anyway.
 */
enum Simple_Scrollable_Background_Entity_Layer {
    SIMPLE_SCROLLABLE_BACKGROUND_ENTITY_LAYER_BACKGROUND = 0,
    SIMPLE_SCROLLABLE_BACKGROUND_ENTITY_LAYER_FOREGROUND = 1,
};
#define SIMPLE_BACKGROUND_WIDTH  (375)
#define SIMPLE_BACKGROUND_HEIGHT (480)
struct Simple_Scrollable_Background_Entity_Main_Data {
    image_id      image_id;
    V2            scale = V2(SIMPLE_BACKGROUND_WIDTH, SIMPLE_BACKGROUND_HEIGHT);
    Simple_Scrollable_Background_Entity_Main_Data* next;
};

struct Simple_Scrollable_Background_Entity_Bundle {
    image_id*      image_id;
    V2*            scale;
    f32*           scroll_y;
    f32*           scroll_x;
    f32*           scroll_speed_y;
    f32*           scroll_speed_x;
};

struct Simple_Scrollable_Background_Entities {
    Simple_Scrollable_Background_Entities() {}
    Simple_Scrollable_Background_Entities(Memory_Arena* arena, s32 count);
    Fixed_Array<Simple_Scrollable_Background_Entity_Main_Data> backgrounds;
    Fixed_Array<f32> scroll_xs;
    Fixed_Array<f32> scroll_ys;
    Fixed_Array<f32> scroll_speed_ys;
    Fixed_Array<f32> scroll_speed_xs;

    void clear(void);
    void update(f32 dt);

    Simple_Scrollable_Background_Entity_Bundle allocate_background(s32 layer);
    Simple_Scrollable_Background_Entity_Bundle get_background(s32 index);

    Simple_Scrollable_Background_Entity_Main_Data* foreground_start, *foreground_end;
    Simple_Scrollable_Background_Entity_Main_Data* background_start, *background_end;

    void draw_foreground(struct render_commands* render_commands, Game_Resources* resources);
    void draw_background(struct render_commands* render_commands, Game_Resources* resources);

private:
    void draw_list(Simple_Scrollable_Background_Entity_Main_Data* list, struct render_commands* render_commands, Game_Resources* resources);
};


struct lua_State;
void bind_entity_lualib(lua_State* L);

#include "entity_prototypes.h"

#endif
