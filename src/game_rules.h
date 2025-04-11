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

#define DEFAULT_SHOW_FLASH_WARNING_TIMER (0.125)
#define DEFAULT_SHOW_FLASH_WARNING_TIMES (5)
#define PLAYER_INVINICIBILITY_TIME   (1.35)
#define PLAYER_GRAZE_RADIUS          (45.0f)

// this needs to be applied in order to avoid abusing
// grazing to rapidly gain score.
#define GRAZING_SCORE_FIRE_RADIUS        (150.0f)  
#define GRAZING_SCORE_AWARD_PICKUP_DELAY (0.100f)
#define PICKUP_ENTITY_AUTO_ATTRACT_DELAY (0.15f)
#define PLAYER_DEFAULT_GRAZING_DELAY (0.075f)
#define GRAZING_SCORE_AWARD_DELAY    (0.050f)
#define GRAZING_DEFAULT_SCORE_AWARD  (100)
#define PICKUP_ENTITY_DEFAULT_LIFETIME (7.4f)
#define PLAYER_BURST_CHARGE_CAPACITY   (100.0f)
#define PLAYER_BURST_FLASH_T           (0.0555f)

/// seconds
#define PLAYER_BURST_SHIELD_ABILITY_RADIUS (60)
#define PLAYER_BURST_SHIELD_ABILITY_MAX_T (4.5f)
#define PLAYER_BURST_SHIELD_COOLDOWN_DEPLETION_APPROX (2.65)
#define PLAYER_BURST_SHIELD_USE_SCORE_AWARD (2500)
#define PLAYER_BURST_SHIELD_REQUIRES_AT_LEAST_ONE_LIFE (true)

/// seconds
#define PLAYER_BURST_RAY_ABILITY_MAX_T    (4.25f)
#define PLAYER_BURST_RAY_COOLDOWN_DEPLETION_APPROX (PLAYER_BURST_RAY_ABILITY_MAX_T*1.30)

/// seconds
#define PLAYER_BURST_BOMB_COOLDOWN_DEPLETION_APPROX (2.75)
#define PLAYER_BURST_BOMB_USE_SCORE_AWARD (7500)
#define PLAYER_BURST_BOMB_REQUIRES_AT_LEAST_ONE_LIFE (true)
#define PLAYER_BURST_BOMB_INVINCIBILITY_T (PLAYER_INVINICIBILITY_TIME * 5)

#define PLAYER_BURST_TIER0_ATTACK_DRAIN_SPEED (28)
#define PLAYER_BURST_TIER1_ATTACK_DRAIN_SPEED (35)
#define PLAYER_BURST_TIER2_ATTACK_DRAIN_SPEED (100)
#define PLAYER_BURST_TIER3_ATTACK_DRAIN_SPEED (170)

#define PLAYER_BURST_TIER0_PASSIVE_DECAY_SPEED (18)
#define PLAYER_BURST_TIER1_PASSIVE_DECAY_SPEED (18)
#define PLAYER_BURST_TIER2_PASSIVE_DECAY_SPEED (10)
#define PLAYER_BURST_TIER3_PASSIVE_DECAY_SPEED (2)

#define PLAYER_DEFAULT_BURST_CHARGE_SPEED (15)

#define INVINCIBILITY_FLASH_TIME_PERIOD (PLAYER_INVINICIBILITY_TIME / 20) / 2
#define DAMAGE_FLASH_TIME_PERIOD        (0.05)
#define ENTITY_TIME_BEFORE_OUT_OF_BOUNDS_DELETION (5.5f)
#define DEFAULT_FIRING_COOLDOWN (0.125)
#define DEFAULT_ENTITY_SCORE_VALUE_PER_HIT (30)
#define DEFAULT_ENTITY_SCORE_KILL_VALUE_MULTIPLIER (5)
#define MAX_PREVIOUS_POSITIONS_FOR_TRAIL (40)
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
};

extern Game_Rules g_game_rules;
extern Game_Rules g_default_game_rules;

Game_Rules serialize_game_rules(struct binary_serializer* serializer);
void       reset_game_rules(void);

#endif
