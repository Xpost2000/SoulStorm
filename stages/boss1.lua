-- All code relating to the first boss encounter
-- meant to be included in a stage file.

--
-- While I'd very much like HEX in the engine,
-- CURSE is probably not worth it, but HEX is
-- 
-- Definitely worth it since this is a stage 1 boss
--
-- The last 15% of hp should be a "PANIC" mode and the
-- boss will just unload everything in a relatively
-- frantic way (this might be a common design pattern I'll use
-- as it is a low-effort way of affectting difficulty)
--

boss1_state = {
    me,
    last_good_position, -- for if the entity is deleted for any reason

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



-- A sort of weird 360 sprout
function Boss1_Sprout1(
    times,
    bloom_pattern_table,
    adjustment_to_i, angstep, angoffset,
    accel, bspeedmin, bspeedmax, basedelay
)
    function max(a, b)
        if a > b then
            return a;
        end
        return b;
    end

    for i=0,times do
        local adji = i+adjustment_to_i;
        Bullet_Pattern_Sprout_Outwards360(
            boss1_state.last_good_position, 
            bloom_pattern_table[1 + i%#bloom_pattern_table], 
            max(bspeedmin + adji * 10, bspeedmax), 
            angstep,
            adji * angoffset,
            1,
            360,
            15, 
            5, 
            accel, -- accel
            7 -- trail count
        );
        t_wait(basedelay - adji*0.025);
    end
end

function Boss1_XCross_Chasing(
    times,
    presenting_visual,
    chasing_visual,
    presentspeed, -- present speed
    firespeed, -- fire speed
    acceleration,  -- acceleration
    trailcount, -- trailcount
    timeuntilchase, -- time until chase
    chaselimit,
    scalewaitfactor
)
    -- Draw X pattern (this is a partial spoke pattern which might be useful to keep)
    scalewaitfactor = scalewaitfactor or 0.45;
    for i=0, times do
        for j=0,4 do
            local bullet = bullet_new(BULLET_SOURCE_ENEMY);
            local position = boss1_state.last_good_position;
            local angle = 90 * (j+1) + 45;
            bullet_set_position(bullet, position[1], position[2]);
            bullet_set_visual(bullet, presenting_visual)
            bullet_set_lifetime(bullet, 20);
            bullet_set_scale(bullet, 5,5);
            bullet_set_visual_scale(bullet, 0.5,0.5);
            bullet_task_lambda(
                bullet,
                function (b)
                    local current_arc_direction = v2_direction_from_degree((angle) % 360);
                    bullet_set_velocity(b, current_arc_direction[1]*presentspeed, current_arc_direction[2]*presentspeed);
                    local tuntilchase = timeuntilchase - (timeuntilchase*scalewaitfactor* i);
                    t_wait(timeuntilchase*scalewaitfactor* i);
                    bullet_set_velocity(b, 0, 0);
                    t_wait(tuntilchase);

                    bullet_start_trail(b, trailcount);
                    bullet_set_trail_modulation(b, 1, 1, 1, 0.3);
                    bullet_set_visual(bullet, chasing_visual);
                    bullet_set_visual_scale(bullet, 0.5,0.5);
                    local spawn_time = bullet_time_since_spawn(b);
                    while true do
                       local dt = bullet_time_since_spawn(b) - spawn_time;
                       local aim = dir_to_player(bullet_position(b));
           
                       aim[1] = aim[1] * (firespeed + (dt * acceleration));
                       aim[2] = aim[2] * (firespeed + (dt * acceleration));
           
                       bullet_set_velocity(b, aim[1], aim[2]);
                       if dt > chaselimit  then
                          break;
                       end
           
                       t_yield();
                    end
                end
            );
        end
    end
end

function Boss1_XCross_RainDown(
    times,
    presenting_visual,
    chasing_visual,
    presentspeed, -- present speed
    firespeed, -- fire speed
    acceleration,  -- acceleration
    trailcount, -- trailcount
    timeuntilchase -- time until chase
)
    -- Draw X pattern (this is a partial spoke pattern which might be useful to keep)
    scalewaitfactor = scalewaitfactor or 0.45;
    for i=0, times do
        for j=0,4 do
            local bullet = bullet_new(BULLET_SOURCE_ENEMY);
            local position = boss1_state.last_good_position;
            local angle = 90 * (j+1) + 45;
            bullet_set_position(bullet, position[1], position[2]);
            bullet_set_visual(bullet, presenting_visual)
            bullet_set_lifetime(bullet, 20);
            bullet_set_scale(bullet, 5,5);
            bullet_set_visual_scale(bullet, 0.5,0.5);
            bullet_task_lambda(
                bullet,
                function (b)
                    local current_arc_direction = v2_direction_from_degree((angle) % 360);
                    bullet_set_velocity(b, current_arc_direction[1]*presentspeed, current_arc_direction[2]*presentspeed);
                    local tuntilchase = timeuntilchase - (timeuntilchase*scalewaitfactor* i);
                    t_wait(timeuntilchase*scalewaitfactor* i);
                    bullet_set_velocity(b, 0, 0);
                    t_wait(tuntilchase);

                    bullet_start_trail(b, trailcount);
                    bullet_set_trail_modulation(b, 1, 1, 1, 0.3);
                    bullet_set_visual(bullet, chasing_visual);
                    bullet_set_visual_scale(bullet, 0.5,0.5);
                    bullet_set_velocity(b, 0, firespeed);
                    bullet_set_acceleration(b, 0, acceleration);
                end
            );
        end
    end
end


-- Chase the player with lasers to force them to move.
function Boss1_GridCrossBoxIn(duration, R, N, VL, HL)
    local spacex = play_area_width()/N;
    local spacey = play_area_height()/N;
    local VL = VL or PROJECTILE_SPRITE_BLUE;
    local HL = HL or PROJECTILE_SPRITE_BLUE;
    for i=0,N do
        laser_hazard_new(i*spacex, R, 0, 0.05, duration, HL);
        laser_hazard_new(i*spacey, R, 1, 0.05, duration, VL);
    end
end

function Boss1_ExplosionChase(explosion_count)
    async_task_lambda(
        function()
            for i=1,laser_count do
                explosion_hazard_new(110, 25, 35, 0.10, 0.10);
                t_wait(0.6);
            end
        end
    )
end

-- Per Frame Logic
function _Stage1_Boss_Logic(eid)
    t_wait(2.5); -- wait out invincibiility.
    boss1_state.last_good_position = enemy_final_position(eid);
    while enemy_valid(eid) do 
        -- Boss1_SelectRain_Attack();
        boss1_state.last_good_position = enemy_final_position(eid);
        -- param pattern 1
        -- Boss1_Sprout1(
        --     8,
        --     {PROJECTILE_SPRITE_BLUE_DISK, PROJECTILE_SPRITE_PURPLE_DISK, PROJECTILE_SPRITE_BLUE_DISK, PROJECTILE_SPRITE_RED_DISK, PROJECTILE_SPRITE_WARM_DISK},
        --     0, -15, prng_ranged_integer(10, 40),
        --     25, 75, 100, 0.5
        -- );

        -- param pattern 2
        -- Boss1_Sprout1(
        --     16,
        --     {PROJECTILE_SPRITE_GREEN_DISK, PROJECTILE_SPRITE_CAUSTIC_DISK, PROJECTILE_SPRITE_GREEN_DISK, PROJECTILE_SPRITE_HOT_PINK_DISK, PROJECTILE_SPRITE_WARM_DISK},
        --     prng_ranged_integer(4, 9), -30, prng_ranged_integer(10, 120),
        --     25, 75, 150, 0.23
        -- );

        -- another attack will be bomb bananza!
        -- each attack will have a specific cooldown associated with it, and movement patterns will not
        -- be synced for any particular attack, the boss just won't move to many places but hopefully the
        -- combination of movements and attacks might lead to interesting "emergent" difficulty.

        -- Whenever I get a hex binder in, only teleports would be used though, and attacking cooldowns
        -- might be increased as hex binders provide passive attacks?

        -- last attack is a crazy spin whirl (like the 360 spin but more erratic)

        -- a variation of this attack should indicate which cells are safe with an explosion
        -- or something highlighting some of them?
        -- Grid 1
        --Boss1_GridCrossBoxIn(5.0, 2, 4, PROJECTILE_SPRITE_GREEN, PROJECTILE_SPRITE_GREEN);
        -- Grid 2
        --Boss1_GridCrossBoxIn(5.0, 3, 3, PROJECTILE_SPRITE_WRM_ELECTRIC, PROJECTILE_SPRITE_WRM_ELECTRIC);
        t_wait(6);
        
        -- Rain 1
        -- Boss1_XCross_RainDown(
        --     6,
        --     PROJECTILE_SPRITE_WRM,
        --     PROJECTILE_SPRITE_WRM_DISK,
        --     55, -- present speed
        --     100, -- fire speed
        --     85,  -- acceleration
        --     5, -- trailcount
        --     1.0 -- time until chase
        -- )
        -- t_wait(5);

        -- XCrossChase 1
        -- Boss1_XCross_Chasing(
        --     9,
        --     PROJECTILE_SPRITE_CAUSTIC,
        --     PROJECTILE_SPRITE_CAUSTIC_DISK,
        --     80, -- present speed
        --     125, -- fire speed
        --     25,  -- acceleration
        --     6, -- trailcount
        --     0.5, -- time until chase
        --     2.25 -- chase for
        -- )
        -- t_wait(5);

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

    Boss1_Sprout1(
        25,
        death_pattern,
        0, 15, 5,
        5, 75, 250, 0.28
    );

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
   enemy_set_hp(e, 525); -- TODO for now
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