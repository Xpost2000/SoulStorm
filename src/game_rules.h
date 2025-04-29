#ifndef GAME_RULES_H
#define GAME_RULES_H

#include "common.h"
#include "serializer.h"

/*
 * Used tuning timing/score values for the game so that
 * replays across versions are "stable" as replays can freeze
 * the ruleset they're using.
 *
 * Since invalidating replays across minor patches that aren't related
 * to stability is suicidial.
 *
 * Does not protect against level/map updates, but I don't think there's
 * a good way to do that anyway...
 *
 * NOTE(jerry):
 * While there's a lot of game rule parameters that are potentially tunable
 * I'm keeping the ones that are the most important to tune, so there are still
 * *some* magic numbers, but most of these magic numbers are derived from these
 * so eh...
 *
 * TODO(jerry):
 * organize these rules
 */

// NOTE(jerry): tickrate is not a rule that needs to be stored.
#define TICKRATE       (60)
#define FIXED_TICKTIME (1.0f / TICKRATE)

///////////////////////////////////////// SUBSTITUTION MACROS
///////////////////////////////////////// TO REDUCE CODE REWRITING....

#define PLAYER_INVINICIBILITY_TIME   (g_game_rules.player_invincibility_time_after_damage)
#define PLAYER_GRAZE_RADIUS          (g_game_rules.player_graze_projectile_radius)

// this needs to be applied in order to avoid abusing
// grazing to rapidly gain score.
#define GRAZING_SCORE_FIRE_RADIUS        (g_game_rules.grazing_score_fire_radius)  
#define GRAZING_SCORE_AWARD_PICKUP_DELAY (g_game_rules.grazing_score_award_pickup_delay)
#define PICKUP_ENTITY_AUTO_ATTRACT_DELAY (g_game_rules.pickup_entity_auto_attract_delay)
#define PLAYER_DEFAULT_GRAZING_DELAY     (g_game_rules.player_default_grazing_delay)
#define GRAZING_SCORE_AWARD_DELAY        (g_game_rules.grazing_score_award_delay)
#define GRAZING_DEFAULT_SCORE_AWARD      (g_game_rules.grazing_default_score_award)
#define PICKUP_ENTITY_DEFAULT_LIFETIME   (g_game_rules.pickup_entity_default_lifetime)
#define PLAYER_BURST_CHARGE_CAPACITY     (g_game_rules.player_burst_charge_capacity)
#define PLAYER_BURST_FLASH_T             (g_game_rules.player_burst_flash_t)

/// seconds
#define PLAYER_BURST_SHIELD_ABILITY_RADIUS             (g_game_rules.player_burst_shield_ability_radius)
#define PLAYER_BURST_SHIELD_ABILITY_MAX_T              (g_game_rules.player_burst_shield_ability_max_t)
#define PLAYER_BURST_SHIELD_COOLDOWN_DEPLETION_APPROX  (g_game_rules.player_burst_shield_cooldown_depletion_approx)
#define PLAYER_BURST_SHIELD_USE_SCORE_AWARD            (g_game_rules.player_burst_shield_use_score_award)
#define PLAYER_BURST_SHIELD_REQUIRES_AT_LEAST_ONE_LIFE (g_game_rules.player_burst_shield_requires_at_least_one_life)

/// seconds
#define PLAYER_BURST_RAY_ABILITY_MAX_T             (g_game_rules.player_burst_ray_ability_max_t)
#define PLAYER_BURST_RAY_COOLDOWN_DEPLETION_APPROX (g_game_rules.player_burst_ray_cooldown_depletion_approx)

/// seconds
#define PLAYER_BURST_BOMB_COOLDOWN_DEPLETION_APPROX  (g_game_rules.player_burst_bomb_cooldown_depletion_approx)
#define PLAYER_BURST_BOMB_USE_SCORE_AWARD            (g_game_rules.player_burst_bomb_use_score_award)
#define PLAYER_BURST_BOMB_REQUIRES_AT_LEAST_ONE_LIFE (g_game_rules.player_burst_bomb_requires_at_least_one_life)
#define PLAYER_BURST_BOMB_INVINCIBILITY_T            (g_game_rules.player_burst_bomb_invincibility_t)

#define PLAYER_BURST_TIER0_ATTACK_DRAIN_SPEED (g_game_rules.player_burst_tier0_attack_drain_speed)
#define PLAYER_BURST_TIER1_ATTACK_DRAIN_SPEED (g_game_rules.player_burst_tier1_attack_drain_speed)
#define PLAYER_BURST_TIER2_ATTACK_DRAIN_SPEED (g_game_rules.player_burst_tier2_attack_drain_speed)
#define PLAYER_BURST_TIER3_ATTACK_DRAIN_SPEED (g_game_rules.player_burst_tier3_attack_drain_speed)

#define PLAYER_BURST_TIER0_PASSIVE_DECAY_SPEED (g_game_rules.player_burst_tier0_passive_decay_speed)
#define PLAYER_BURST_TIER1_PASSIVE_DECAY_SPEED (g_game_rules.player_burst_tier1_passive_decay_speed)
#define PLAYER_BURST_TIER2_PASSIVE_DECAY_SPEED (g_game_rules.player_burst_tier2_passive_decay_speed)
#define PLAYER_BURST_TIER3_PASSIVE_DECAY_SPEED (g_game_rules.player_burst_tier3_passive_decay_speed)

#define PLAYER_DEFAULT_BURST_CHARGE_SPEED          (g_game_rules.player_default_burst_charge_speed)
#define DEFAULT_FIRING_COOLDOWN                    (g_game_rules.default_firing_cooldown)
#define DEFAULT_ENTITY_SCORE_VALUE_PER_HIT         (g_game_rules.default_entity_score_value_per_hit)
#define DEFAULT_ENTITY_SCORE_KILL_VALUE_MULTIPLIER (g_game_rules.default_entity_score_kill_value_multiplier)

// These aren't really rules that matter much, as these are purely visual.
#define INVINCIBILITY_FLASH_TIME_PERIOD (PLAYER_INVINICIBILITY_TIME / 20) / 2
#define DAMAGE_FLASH_TIME_PERIOD        (0.05)
#define ENTITY_TIME_BEFORE_OUT_OF_BOUNDS_DELETION (5.5f) // this will never change.
// visual only
#define MAX_PREVIOUS_POSITIONS_FOR_TRAIL (40)
#define DEFAULT_SHOW_FLASH_WARNING_TIMER (0.125)
#define DEFAULT_SHOW_FLASH_WARNING_TIMES (5)
// not changing.
#define PICKUP_ENTITY_ANIMATION_T_LENGTH (0.75f)
#define PICKUP_ENTITY_FADE_T_LENGTH      (0.10f)

/*
 * NOTE(jerry):
 *
 * Gameplay *should* not really change so I am not supposed to need
 * more rules, but just in-case...
 */
#define GAME_RULES_VERSION_0       (0)
#define GAME_RULES_VERSION_CURRENT (GAME_RULES_VERSION_0)
struct Game_Rules {
    s32 version = GAME_RULES_VERSION_CURRENT;

    f32 player_invincibility_time_after_damage = 1.35f;
    f32 player_graze_projectile_radius         = 45.0f; // pixels

    f32 grazing_score_fire_radius              = 150.0f;
    f32 grazing_score_award_pickup_delay       = 0.100f;
    f32 pickup_entity_auto_attract_delay       = 0.15f;
    f32 player_default_grazing_delay           = 0.075f;
    f32 grazing_score_award_delay              = 0.050f;
    s32 grazing_default_score_award            = 100;
    f32 pickup_entity_default_lifetime         = 7.4f;
    f32 player_burst_charge_capacity           = 100.0f; //... why is this a float?
    f32 player_burst_flash_t                   = 0.0555f;

    f32 player_burst_shield_ability_radius             = 60.0f;
    f32 player_burst_shield_ability_max_t              = 4.5f;
    f32 player_burst_shield_cooldown_depletion_approx  = 2.65f;
    s32 player_burst_shield_use_score_award            = 2500;
    s32 player_burst_shield_requires_at_least_one_life = 0;

    f32 player_burst_ray_ability_max_t             = 4.25f;
    f32 player_burst_ray_cooldown_depletion_approx = 3.00f;

    f32 player_burst_bomb_cooldown_depletion_approx  = 2.75f;
    s32 player_burst_bomb_use_score_award            = 7500;
    s32 player_burst_bomb_requires_at_least_one_life = 1;
    f32 player_burst_bomb_invincibility_t            = 1.35f * 5.0f;

    s32 player_burst_tier0_attack_drain_speed        = 35;
    s32 player_burst_tier1_attack_drain_speed        = 35;
    s32 player_burst_tier2_attack_drain_speed        = 100;
    s32 player_burst_tier3_attack_drain_speed        = 170;

    s32 player_burst_tier0_passive_decay_speed        = 18;
    s32 player_burst_tier1_passive_decay_speed        = 18;
    s32 player_burst_tier2_passive_decay_speed        = 10;
    s32 player_burst_tier3_passive_decay_speed        = 2;
    s32 player_default_burst_charge_speed             = 2;

    f32 default_firing_cooldown = 0.125f;
    s32 default_entity_score_value_per_hit = 30;
    s32 default_entity_score_kill_value_multiplier = 5;

    s32    pet_data_none_lives          = 4; // MAX_BASE_TRIES
    f32    pet_data_none_score_modifier = 1.0f;
    f32    pet_data_none_speed_modifier = 1.0f;

    s32    pet_data_cat_lives          = 9;
    f32    pet_data_cat_score_modifier = 0.35f;
    f32    pet_data_cat_speed_modifier = 1.0f;

    s32    pet_data_dog_lives          = 3;
    f32    pet_data_dog_score_modifier = 1.5f;
    f32    pet_data_dog_speed_modifier = 1.25f;

    s32    pet_data_fish_lives          = 1;
    f32    pet_data_fish_score_modifier = 4.00f;
    f32    pet_data_fish_speed_modifier = 0.875f;
};

extern Game_Rules g_game_rules;
extern const Game_Rules g_default_game_rules;

Game_Rules serialize_game_rules(struct binary_serializer* serializer);
void       reset_game_rules(void);

#endif
