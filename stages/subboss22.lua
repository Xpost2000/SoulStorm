-- Handful of attack patterns but meant to be pretty short...

boss_state = {
    me,
    last_good_position, -- for if the entity is deleted for any reason
    starting_position,

    -- general boss logic state
    next_rain_attack_until = -9999,

    -- Attacking actions. Attack actions and movements
    -- are scheduled "simulatneously"
    next_think_action_t = -9999,
};

BOSS_HP = 3500;

function Game_Spawn_Stage2_2_SubBoss()
    local e = enemy_new();
    local initial_boss_pos = v2(play_area_width()/2, 50);
    enemy_set_hp(e, BOSS_HP); -- TODO for now
    enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
    enemy_set_scale(e, 50, 50); -- chunky.
    -- enemy_set_visual(e, ENTITY_SPRITE_BOSS1);
    enemy_set_burst_gain_value(e, 0.55);
    enemy_show_boss_hp(e, "SKELETON ANGEL");
    -- The boss takes "three threads" of logic.
    async_task_lambda(_Boss_Logic, e);
    -- async_task_lambda(_Boss_Movement_Logic, e);
    async_task_lambda(_Boss_ImmunityToBurstLaser_Logic, e);
    -- async_task_lambda(_Boss_MusicPlayer, e);
    boss_state.me = e;
    boss_state.starting_position = initial_boss_pos;
 
    return e;
 end