#include "game_rules.h"

/*
 * This is something that should really be metaprogrammed, but
 * I don't really want to expend too much effort doing this...
 */

Game_Rules g_game_rules = {};
const Game_Rules g_default_game_rules = {};

static void print_labelled(const char* labelled, f32 f)
{
    _debugprintf("%s: f32 = %f", labelled, f);
}

static void print_labelled(const char* labelled, s32 f)
{
    _debugprintf("%s: s32 = %d", labelled, f);
}

#define STRINGIFY(x) #x
#define DisplayRule(rules, x) print_labelled(STRINGIFY(x), rules.x)

static void debug_show_game_rules(Game_Rules& rules)
{
    DisplayRule(rules, version);
    DisplayRule(rules, player_invincibility_time_after_damage);
    DisplayRule(rules, player_graze_projectile_radius);
    DisplayRule(rules, grazing_score_fire_radius);
    DisplayRule(rules, grazing_score_award_pickup_delay);
    DisplayRule(rules, pickup_entity_auto_attract_delay);
    DisplayRule(rules, player_default_grazing_delay);
    DisplayRule(rules, grazing_score_award_delay);
    DisplayRule(rules, grazing_default_score_award);
    DisplayRule(rules, pickup_entity_default_lifetime);
    DisplayRule(rules, player_burst_charge_capacity);
    DisplayRule(rules, player_burst_flash_t);
    DisplayRule(rules, player_burst_shield_ability_radius);
    DisplayRule(rules, player_burst_shield_ability_max_t);
    DisplayRule(rules, player_burst_shield_cooldown_depletion_approx);
    DisplayRule(rules, player_burst_shield_use_score_award);
    DisplayRule(rules, player_burst_shield_requires_at_least_one_life);
    DisplayRule(rules, player_burst_ray_ability_max_t);
    DisplayRule(rules, player_burst_ray_cooldown_depletion_approx);
    DisplayRule(rules, player_burst_bomb_cooldown_depletion_approx);
    DisplayRule(rules, player_burst_bomb_use_score_award);
    DisplayRule(rules, player_burst_bomb_requires_at_least_one_life);
    DisplayRule(rules, player_burst_bomb_invincibility_t);
    DisplayRule(rules, player_burst_tier0_attack_drain_speed);
    DisplayRule(rules, player_burst_tier1_attack_drain_speed);
    DisplayRule(rules, player_burst_tier2_attack_drain_speed);
    DisplayRule(rules, player_burst_tier3_attack_drain_speed);
    DisplayRule(rules, player_burst_tier0_passive_decay_speed);
    DisplayRule(rules, player_burst_tier1_passive_decay_speed);
    DisplayRule(rules, player_burst_tier2_passive_decay_speed);
    DisplayRule(rules, player_burst_tier3_passive_decay_speed);
    DisplayRule(rules, player_default_burst_charge_speed);
    DisplayRule(rules, default_firing_cooldown);
    DisplayRule(rules, default_entity_score_value_per_hit);
    DisplayRule(rules, default_entity_score_kill_value_multiplier);

    DisplayRule(rules, pet_data_none_lives);
    DisplayRule(rules, pet_data_none_score_modifier);
    DisplayRule(rules, pet_data_none_speed_modifier);

    DisplayRule(rules, pet_data_cat_lives);
    DisplayRule(rules, pet_data_cat_score_modifier);
    DisplayRule(rules, pet_data_cat_speed_modifier);

    DisplayRule(rules, pet_data_dog_lives);
    DisplayRule(rules, pet_data_dog_score_modifier);
    DisplayRule(rules, pet_data_dog_speed_modifier);

    DisplayRule(rules, pet_data_fish_lives);
    DisplayRule(rules, pet_data_fish_score_modifier);
    DisplayRule(rules, pet_data_fish_speed_modifier);

    DisplayRule(rules, continue_count);
}

Game_Rules serialize_game_rules(struct binary_serializer* serializer)
{
    Game_Rules result = {};
    serialize_s32(serializer, &result.version);
    switch (result.version) {
        case GAME_RULES_VERSION_0: {
            serialize_f32(serializer, &result.player_invincibility_time_after_damage);
            serialize_f32(serializer, &result.player_graze_projectile_radius);
            serialize_f32(serializer, &result.grazing_score_fire_radius);
            serialize_f32(serializer, &result.grazing_score_award_pickup_delay);
            serialize_f32(serializer, &result.pickup_entity_auto_attract_delay);
            serialize_f32(serializer, &result.player_default_grazing_delay);
            serialize_f32(serializer, &result.grazing_score_award_delay);
            serialize_s32(serializer, &result.grazing_default_score_award);
            serialize_f32(serializer, &result.pickup_entity_default_lifetime);
            serialize_f32(serializer, &result.player_burst_charge_capacity);
            serialize_f32(serializer, &result.player_burst_flash_t);

            serialize_f32(serializer, &result.player_burst_shield_ability_radius);
            serialize_f32(serializer, &result.player_burst_shield_ability_max_t);
            serialize_f32(serializer, &result.player_burst_shield_cooldown_depletion_approx);
            serialize_s32(serializer, &result.player_burst_shield_use_score_award);
            serialize_s32(serializer, &result.player_burst_shield_requires_at_least_one_life);

            serialize_f32(serializer, &result.player_burst_ray_ability_max_t);
            serialize_f32(serializer, &result.player_burst_ray_cooldown_depletion_approx);

            serialize_f32(serializer, &result.player_burst_bomb_cooldown_depletion_approx);
            serialize_s32(serializer, &result.player_burst_bomb_use_score_award);
            serialize_s32(serializer, &result.player_burst_bomb_requires_at_least_one_life);
            serialize_f32(serializer, &result.player_burst_bomb_invincibility_t);

            serialize_s32(serializer, &result.player_burst_tier0_attack_drain_speed);
            serialize_s32(serializer, &result.player_burst_tier1_attack_drain_speed);
            serialize_s32(serializer, &result.player_burst_tier2_attack_drain_speed);
            serialize_s32(serializer, &result.player_burst_tier3_attack_drain_speed);

            serialize_s32(serializer, &result.player_burst_tier0_passive_decay_speed);
            serialize_s32(serializer, &result.player_burst_tier1_passive_decay_speed);
            serialize_s32(serializer, &result.player_burst_tier2_passive_decay_speed);
            serialize_s32(serializer, &result.player_burst_tier3_passive_decay_speed);
            serialize_s32(serializer, &result.player_default_burst_charge_speed);

            serialize_f32(serializer, &result.default_firing_cooldown);
            serialize_s32(serializer, &result.default_entity_score_value_per_hit);
            serialize_s32(serializer, &result.default_entity_score_kill_value_multiplier);

            serialize_s32(serializer, &result.pet_data_none_lives);
            serialize_s32(serializer, &result.pet_data_cat_lives);
            serialize_s32(serializer, &result.pet_data_dog_lives);
            serialize_s32(serializer, &result.pet_data_fish_lives);

            serialize_f32(serializer, &result.pet_data_none_score_modifier);
            serialize_f32(serializer, &result.pet_data_none_speed_modifier);

            serialize_f32(serializer, &result.pet_data_cat_score_modifier);
            serialize_f32(serializer, &result.pet_data_cat_speed_modifier);

            serialize_f32(serializer, &result.pet_data_dog_score_modifier);
            serialize_f32(serializer, &result.pet_data_dog_speed_modifier);

            serialize_f32(serializer, &result.pet_data_fish_score_modifier);
            serialize_f32(serializer, &result.pet_data_fish_speed_modifier);
        } break;
        case GAME_RULES_VERSION_1: {
          serialize_f32(serializer, &result.player_invincibility_time_after_damage);
          serialize_f32(serializer, &result.player_graze_projectile_radius);
          serialize_f32(serializer, &result.grazing_score_fire_radius);
          serialize_f32(serializer, &result.grazing_score_award_pickup_delay);
          serialize_f32(serializer, &result.pickup_entity_auto_attract_delay);
          serialize_f32(serializer, &result.player_default_grazing_delay);
          serialize_f32(serializer, &result.grazing_score_award_delay);
          serialize_s32(serializer, &result.grazing_default_score_award);
          serialize_f32(serializer, &result.pickup_entity_default_lifetime);
          serialize_f32(serializer, &result.player_burst_charge_capacity);
          serialize_f32(serializer, &result.player_burst_flash_t);

          serialize_f32(serializer, &result.player_burst_shield_ability_radius);
          serialize_f32(serializer, &result.player_burst_shield_ability_max_t);
          serialize_f32(serializer, &result.player_burst_shield_cooldown_depletion_approx);
          serialize_s32(serializer, &result.player_burst_shield_use_score_award);
          serialize_s32(serializer, &result.player_burst_shield_requires_at_least_one_life);

          serialize_f32(serializer, &result.player_burst_ray_ability_max_t);
          serialize_f32(serializer, &result.player_burst_ray_cooldown_depletion_approx);

          serialize_f32(serializer, &result.player_burst_bomb_cooldown_depletion_approx);
          serialize_s32(serializer, &result.player_burst_bomb_use_score_award);
          serialize_s32(serializer, &result.player_burst_bomb_requires_at_least_one_life);
          serialize_f32(serializer, &result.player_burst_bomb_invincibility_t);

          serialize_s32(serializer, &result.player_burst_tier0_attack_drain_speed);
          serialize_s32(serializer, &result.player_burst_tier1_attack_drain_speed);
          serialize_s32(serializer, &result.player_burst_tier2_attack_drain_speed);
          serialize_s32(serializer, &result.player_burst_tier3_attack_drain_speed);

          serialize_s32(serializer, &result.player_burst_tier0_passive_decay_speed);
          serialize_s32(serializer, &result.player_burst_tier1_passive_decay_speed);
          serialize_s32(serializer, &result.player_burst_tier2_passive_decay_speed);
          serialize_s32(serializer, &result.player_burst_tier3_passive_decay_speed);
          serialize_s32(serializer, &result.player_default_burst_charge_speed);

          serialize_f32(serializer, &result.default_firing_cooldown);
          serialize_s32(serializer, &result.default_entity_score_value_per_hit);
          serialize_s32(serializer, &result.default_entity_score_kill_value_multiplier);

          serialize_s32(serializer, &result.pet_data_none_lives);
          serialize_s32(serializer, &result.pet_data_cat_lives);
          serialize_s32(serializer, &result.pet_data_dog_lives);
          serialize_s32(serializer, &result.pet_data_fish_lives);

          serialize_f32(serializer, &result.pet_data_none_score_modifier);
          serialize_f32(serializer, &result.pet_data_none_speed_modifier);

          serialize_f32(serializer, &result.pet_data_cat_score_modifier);
          serialize_f32(serializer, &result.pet_data_cat_speed_modifier);

          serialize_f32(serializer, &result.pet_data_dog_score_modifier);
          serialize_f32(serializer, &result.pet_data_dog_speed_modifier);

          serialize_f32(serializer, &result.pet_data_fish_score_modifier);
          serialize_f32(serializer, &result.pet_data_fish_speed_modifier);

          serialize_s32(serializer, &result.continue_count);
        } break;
        default: {
            _debugprintf("Unknown game rules structure version. Using default game rules.");
            result = g_default_game_rules;
        } break;
    }

    debug_show_game_rules(result);
    return result;
}

void reset_game_rules(void)
{
    g_game_rules = g_default_game_rules;
}
