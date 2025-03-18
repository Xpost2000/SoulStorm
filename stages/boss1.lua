-- All code relating to the first boss encounter
-- meant to be included in a stage file.

boss1_state = {
    me,

    -- general boss logic state
    next_rain_attack_until = -9999,
    next_think_action_t = -9999,

    -- attack pattern state specifics
    hexbind0=0,
    hexbind1=0,
};

hexbind0_state = {

};

hexbind1_state = {

};

function Boss1_SelectRain_Attack()
    local boss_health_percentage = enemy_hp_percent(boss1_state.me);
    local atk_t = enemy_time_since_spawn(boss1_state.me);
    
    if atk_t < boss1_state.next_rain_attack_until then
        print("Not ready to attack")
        return;
    end

    local seed = prng_ranged_integer(1000, 9999);
    if boss_health_percentage >= 0.5 then
        local atkduration = prng_ranged_float(5.5, 7,5);
        MainBoss1_RainCloud_Attack1(seed, atkduration);
        boss1_state.next_rain_attack_until = atk_t + atkduration + prng_ranged_float(3.5, 8.5);
    else
        local atkduration = prng_ranged_float(7.5, 8.5);
        MainBoss1_RainCloud_Attack2(seed, atkduration);
        boss1_state.next_rain_attack_until = atk_t + atkduration + prng_ranged_float(5.5, 7.5);
    end
end

-- Per Frame Logic
function _Stage1_Boss_Logic(eid)
    t_wait(2.5); -- wait out invincibiility.
    local eposition = enemy_final_position(eid);
    while enemy_valid(eid) do 
        -- Boss1_SelectRain_Attack();
        eposition = enemy_final_position(eid);
        t_yield();
    end
    -- we have latency of 1 dt after death :/
    -- for angle=1,360,30 do
    --     local bullet = bullet_new(BULLET_SOURCE_ENEMY);
    --     bullet_set_position(bullet, eposition[1], eposition[2]);
    --     bullet_set_visual(bullet, PROJECTILE_SPRITE_CAUSTIC_DISK)
    --     bullet_set_lifetime(bullet, 15);
    --     bullet_set_scale(bullet, 5, 5);
    --     bullet_set_visual_scale(bullet, 0.5, 0.5);

    --     local bdir = v2_direction_from_degree(angle);
    --     bullet_set_velocity(bullet, bdir[1] * 45, bdir[2] * 45);
    --     bullet_start_trail(bullet, 4);
    --  end
    local death_pattern = {
        PROJECTILE_SPRITE_CAUSTIC_DISK, 
        PROJECTILE_SPRITE_BLUE_DISK,
        PROJECTILE_SPRITE_RED_DISK,
        PROJECTILE_SPRITE_PURPLE_DISK
    };

    function max(a, b)
        if a > b then
            return a;
        end
        return b;
    end

    for i=0,25 do
        Bullet_Pattern_Sprout_Outwards360(
            eposition, 
            death_pattern[1 + i%4], 
            max(75 + i * 10, 150), 
            15,
            i * 5,
            1,
            360,
            15, 
            5, 
            5,
            7
         );
         t_wait(0.29 - i*0.025);
    end

    -- do some explosions and stuff for
    -- a dramatic death.
    --
    -- Stage end.
    --
    -- Bosses, don't really do anything special
    -- when the stage ends other than have a dramatic
    -- explosion (that is dangerous btw, so don't get complicant!)
    --
    -- Flashy, and deadly.
    explosion_hazard_new(50, 50, 25, 0.00, 0.00);
    explosion_hazard_new(play_area_width()-50, 50, 25, 0.25, 0.25);
    t_wait(0.25);
    explosion_hazard_new(75, 65, 35, 0.10, 0.10);
    t_wait(0.15);
    explosion_hazard_new(play_area_width()-95, 45, 35, 0.10, 0.10);
    t_wait(0.15);
    explosion_hazard_new(110, 25, 35, 0.10, 0.10);
    t_wait(0.15);
    explosion_hazard_new(play_area_width()-100, 25, 35, 0.10, 0.10);
    t_wait(0.15);
    explosion_hazard_new(140, 45, 35, 0.15, 0.15);
    t_wait(0.35);
    explosion_hazard_new(75, 65, 35, 0.10, 0.10);
    t_wait(0.25);
    explosion_hazard_new(play_area_width()-95, 45, 35, 0.10, 0.10);
    t_wait(0.25);
    explosion_hazard_new(110, 25, 35, 0.10, 0.10);
    t_wait(1.25);

    while enemy_valid(boss1_state.hexbind0) or enemy_valid(boss1_state.hexbind1) do 
        t_yield();
    end
    t_complete_stage();
end

function _Stage1_Boss_HexBind0(eid)
end

function _Stage1_Boss_HexBind1(eid)
end

function Game_Spawn_Stage1_Boss()
   local e = enemy_new();
   local initial_boss_pos = v2(play_area_width()/2, 50);
   enemy_set_hp(e, 100); -- TODO for now
   enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
   enemy_set_visual(e, ENTITY_SPRITE_BAT_A);
   enemy_show_boss_hp(e, "WITCH");
   async_task_lambda(_Stage1_Boss_Logic, e);
   boss1_state.me = e;
   return e;
end

function Game_Spawn_Stage1_Boss_HexBind0()
    local e = enemy_new();
    local initial_boss_pos = v2(play_area_width()/2 + 50, 50);
    enemy_set_hp(e, 100);
    enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
    enemy_set_visual(e, ENTITY_SPRITE_SKULL_A); -- for now...
    enemy_show_boss_hp(e, "HEX BINDING");
    async_task_lambda(_Stage1_Boss_HexBind0, e);
    return e;
 end
 
 function Game_Spawn_Stage1_Boss_HexBind1()
    local e = enemy_new();
    local initial_boss_pos = v2(play_area_width()/2 - 50, 50);
    enemy_set_hp(e, 100);
    enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
    enemy_set_visual(e, ENTITY_SPRITE_SKULL_A); -- for now...
    enemy_show_boss_hp(e, "HEX BINDING");
    async_task_lambda(_Stage1_Boss_HexBind1, e);
    return e;
 end