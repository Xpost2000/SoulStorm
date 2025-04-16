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
-- TODO(jerry)
--
-- I think the fight mechanics are fine, but I need to give the player more breathing room.
-- 
local track1 = load_music("res/snds/music_1_3_bossloop.ogg");

enable_boss_death_explosion = false;
BOSS_HP = 500;
MINI_BOSS_HP = 100;
BOSS_HP_LOSS_FROM_HEX_FINISH = BOSS_HP/10;

boss1_teleport_positions = {}; -- forward declaration
boss1_state = {
    me,
    last_good_position, -- for if the entity is deleted for any reason
    starting_position,

    -- general boss logic state
    next_rain_attack_until = -9999,

    -- Attacking actions. Attack actions and movements
    -- are scheduled "simulatneously"
    next_think_action_t = -9999,
    next_grid_action_t = -9999,

    -- Not always moving, but sometimes move...
    next_possible_move_action_t = -9999,

    -- attack pattern state specifics
    hexbind0=0,
    hexbind1=0,
    hexbind2=0,
    hexbind3=0,

    -- PHASE 0 default
    -- PHASE 1 HEX BINDER WAVE 1 -- 75% hp
    -- PHASE 2 WAIT UNTIL BINDER 1 DEATH
    -- These hexbinders do not move really
    -- PHASE 3 HEX BINDER WAVE 2 -- 45% hp
    -- PHASE 4 WAIT UNTIL BINDER 2 DEATH
    -- These hexbinders will move around
    -- PHASE 5 HEX BINDER WAVE 3 -- 25% hp
    -- These hexbinders are static, the numbers are overwhelming
    -- PHASE 6 PANIC LAST STAND  -- 15% hp
    phase=0,

    -- movement data
    wants_to_move=false,
    moving_to=false,
    move_target,
    move_method='teleport',
    move_speed, -- Velocity if not teleporting
    -- explosion speed is fixed size
};

hexbind0_state = {
   id=0,
   next_think_action_t = -9999,
};
hexbind1_state = {
   id=1,
   next_think_action_t = -9999,
};
hexbind2_state = {
   id=2,
   next_think_action_t = -9999,
};
hexbind3_state = {
   id=3,
   next_think_action_t = -9999,
};

BOSS1_ALERT_STAY_LENGTH = 3.76;
function Boss1_Begin_Hex()
   show_gameplay_alert("PLAYER HEXED!", MENU_FONT_COLOR_ORANGE, BOSS1_ALERT_STAY_LENGTH);
   show_damage_player_will_take();
   player_set_damage_per_hit(2);
end

function Boss1_Begin_Curse()
   show_gameplay_alert("PLAYER CURSED!", MENU_FONT_COLOR_ORANGE, BOSS1_ALERT_STAY_LENGTH);
   show_damage_player_will_take();
   player_set_damage_per_hit(player_hp());
end

function Boss1_End_Hex()
   show_gameplay_alert("HEX ENDED!", MENU_FONT_COLOR_PURPLE, BOSS1_ALERT_STAY_LENGTH);
   hide_damage_player_will_take();
   player_set_damage_per_hit(1);
end

function Boss1_End_Curse()
   show_gameplay_alert("CURSE ENDED!", MENU_FONT_COLOR_PURPLE, BOSS1_ALERT_STAY_LENGTH);
   hide_damage_player_will_take();
   player_set_damage_per_hit(1);
end

function Boss1_Enemy_HexShield(eid)
   local particle_emitter_ptr = enemy_get_particle_emitter(eid, 0);
   particle_emitter_set_active(particle_emitter_ptr, true);
   particle_emitter_set_lifetime(particle_emitter_ptr, 1.0);
   particle_emitter_set_scale(particle_emitter_ptr, 0.1);
   particle_emitter_set_max_emissions(particle_emitter_ptr, -1);
   particle_emitter_set_emit_per_emission(particle_emitter_ptr, 26);
   particle_emitter_set_emission_max_timer(particle_emitter_ptr, 0.01);
   particle_emitter_set_scale_variance(particle_emitter_ptr, 0.05, 0.2);
   particle_emitter_set_use_angular(particle_emitter_ptr, true);
   particle_emitter_set_angle_range(particle_emitter_ptr, -360, 360);
   particle_emitter_set_velocity_y_variance(particle_emitter_ptr, 150, 250);
   particle_emitter_set_velocity_x_variance(particle_emitter_ptr, -100, 100);
   particle_emitter_set_acceleration(particle_emitter_ptr, 0, 98);
   particle_emitter_set_velocity(particle_emitter_ptr, 10, 20);
   particle_emitter_set_modulation(particle_emitter_ptr, 1, 1, 1, 0.25);
   particle_emitter_set_use_flame_mode(particle_emitter_ptr, true);
   local initial_boss_pos = enemy_final_position(eid);

   particle_emitter_set_emit_shape_circle(particle_emitter_ptr, initial_boss_pos[1], initial_boss_pos[2], 30, false);
   particle_emitter_set_sprite_projectile(particle_emitter_ptr, PROJECTILE_SPRITE_PURPLE_ELECTRIC);
end

function Boss1_Enemy_HexAttractor(eid, attract_to_eid)
   local particle_emitter_ptr = enemy_get_particle_emitter(eid, 0);
   particle_emitter_set_active(particle_emitter_ptr, true);
   particle_emitter_set_lifetime(particle_emitter_ptr, 4);
   particle_emitter_set_scale(particle_emitter_ptr, 0.05);
   particle_emitter_set_max_emissions(particle_emitter_ptr, -1);
   particle_emitter_set_emit_per_emission(particle_emitter_ptr, 8);
   particle_emitter_set_emission_max_timer(particle_emitter_ptr, 0.01);
   particle_emitter_set_scale_variance(particle_emitter_ptr, 0.02, 0.1);
   
   -- particle_emitter_set_use_angular(particle_emitter_ptr, true);
   -- particle_emitter_set_angle_range(particle_emitter_ptr, -360, 360);
   -- particle_emitter_set_velocity_y_variance(particle_emitter_ptr, 150, 250);
   -- particle_emitter_set_velocity_x_variance(particle_emitter_ptr, -100, 100);
   -- particle_emitter_set_acceleration(particle_emitter_ptr, 0, 98);
   -- particle_emitter_set_velocity(particle_emitter_ptr, 10, 20);

   particle_emitter_set_modulation(particle_emitter_ptr, 1, 1, 1, 0.25);
   particle_emitter_set_use_flame_mode(particle_emitter_ptr, true);
   particle_emitter_set_use_attraction_point(particle_emitter_ptr, true);
   local initial_boss_pos = enemy_final_position(eid);
   local attract_to_pos = enemy_final_position(attract_to_eid);

   particle_emitter_set_attraction_force(particle_emitter_ptr, 500);
   particle_emitter_set_attraction_point(particle_emitter_ptr, attract_to_pos[1], attract_to_pos[2]);

   particle_emitter_set_emit_shape_circle(particle_emitter_ptr, initial_boss_pos[1], initial_boss_pos[2], 2, false);
   particle_emitter_set_sprite_projectile(particle_emitter_ptr, PROJECTILE_SPRITE_BLUE_ELECTRIC);
end


function Boss1_Schedule_Teleport_To(position, force)
   force = force or false;

   if boss1_state.wants_to_move or boss1_state.moving_to then
      if not force then
         return false;
      end
   end

   if (v2_equal(position, boss1_state.last_good_position)) then
      -- do not schedule movement that is redundant.
      return false;
   end

   boss1_state.wants_to_move = true;
   boss1_state.moving_to = false;
   boss1_state.move_method = 'teleport';
   boss1_state.move_target = position;
   return true;
end

function Boss1_SelectRain_Attack()
    local boss_health_percentage = enemy_hp_percent(boss1_state.me);
    local atk_t = enemy_time_since_spawn(boss1_state.me);
    
    if atk_t < boss1_state.next_rain_attack_until then
        print("Not ready to attack with rain")
        return false;
    end

    local seed = prng_ranged_integer(1000, 9999);
    if boss_health_percentage >= 0.5 then
        local atkduration = prng_ranged_float(5.5, 7,5);
        MainBoss1_RainCloud_Attack1(seed, atkduration, 0.4, 32);
        boss1_state.next_rain_attack_until = atk_t + atkduration + prng_ranged_float(3.5, 8.5);
    else
        local atkduration = prng_ranged_float(7.5, 8.5);
        MainBoss1_RainCloud_Attack2(seed, atkduration, 0.46, 34);
        boss1_state.next_rain_attack_until = atk_t + atkduration + prng_ranged_float(8.5, 12.5);
    end

    return true;
end



-- A sort of weird 360 sprout
function Boss1_Sprout1(
    position,
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
      play_sound(random_attack_sound());

        local adji = i+adjustment_to_i;
        Bullet_Pattern_Sprout_Outwards360(
            position, 
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
         play_sound(random_attack_sound());
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
    position,
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
            explosion_hazard_new(player_position_x(), player_position_y(), 32, 0.15, 0.02);
            for i=1,explosion_count do
                explosion_hazard_new(player_position_x(), player_position_y(), 32, 0.045, 0.05);
                t_wait(0.43);
            end
        end
    )
end

function remaining_hexbind_minions()
   return enemy_valid(boss1_state.hexbind0) or enemy_valid(boss1_state.hexbind1) or
      enemy_valid(boss1_state.hexbind2) or enemy_valid(boss1_state.hexbind3);
end

-- Per Frame Logic
function _Stage1_Boss_Logic(eid)
    t_wait(2.5); -- wait out invincibiility.
    boss1_state.last_good_position = enemy_final_position(eid);
    while enemy_valid(eid) do 
        local boss_health_percentage = enemy_hp_percent(boss1_state.me);
        boss1_state.last_good_position = enemy_final_position(eid);

        if boss_health_percentage <= 0.10 then
            disable_grazing();
            start_black_fade(0.065);
            -- teleport back to center first then
            -- do panic attack until death

            if (not v2_equal(boss1_state.last_good_position, boss1_state.starting_position)) then
               enemy_begin_invincibility(boss1_state.me, true, 2);
               Boss1_Schedule_Teleport_To(
                  boss1_state.starting_position
               );
            else
               -- param pattern 3
               for i=0,4 do
                  Boss1_Sprout1(
                     boss1_state.last_good_position,
                     prng_ranged_integer(6, 7),
                     {PROJECTILE_SPRITE_RED_DISK, PROJECTILE_SPRITE_GREEN_DISK, PROJECTILE_SPRITE_BLUE_DISK, PROJECTILE_SPRITE_PURPLE_DISK, PROJECTILE_SPRITE_CAUSTIC_DISK},
                     prng_ranged_integer(6, 9), 30, prng_ranged_integer(15, 120),
                     5, 60, 110, 0.21
                  );
                  t_wait(prng_ranged_float(0.125, 0.35));
                  Boss1_Sprout1(
                     boss1_state.last_good_position,
                     prng_ranged_integer(6, 8),
                     {PROJECTILE_SPRITE_RED_DISK, PROJECTILE_SPRITE_GREEN_DISK, PROJECTILE_SPRITE_BLUE_DISK, PROJECTILE_SPRITE_PURPLE_DISK, PROJECTILE_SPRITE_CAUSTIC_DISK},
                     prng_ranged_integer(5, 12), 50, prng_ranged_integer(50, 90),
                     5, 60, 120, 0.21
                  );
               end
            end
        else
           local current_t = enemy_time_since_spawn(boss1_state.me);

           -- Movement actions
           if (current_t >= boss1_state.next_possible_move_action_t) then
              boss1_state.next_possible_move_action_t = current_t + prng_ranged_float(2.5, 4.5);
              local action_rng = prng_ranged_integer(0, 100);

              -- 45% chance to move normally
              if action_rng < 45 then
                 Boss1_Schedule_Teleport_To(
                    boss1_teleport_positions[prng_ranged_integer(0, #boss1_teleport_positions-1)]
                 );
              end
           end

           -- Default Attack Patterns (until 75%)
           -- TODO: add below 75% patterns which are just
           -- faster and a few more.
           if (current_t >= boss1_state.next_think_action_t) then
              -- next think action depends on what attack was chosen
              local action_rng = prng_ranged_integer(0, 100);

              -- 75% of attack
              if action_rng < 75 then
                 action_rng = prng_ranged_integer(0, 13);
                 -- action_rng =3;

                 -- Attack ID
                 if action_rng == 0 or action_rng == 12 or action_rng == 13 then
                    if (Boss1_SelectRain_Attack()) then
                       -- ?
                    end
                    boss1_state.next_think_action_t = current_t + prng_ranged_float(3.5, 4.5);
                 end

                 if action_rng == 1 or action_rng == 11 then
                    Boss1_Sprout1(
                     boss1_state.last_good_position,
                     8,
                       {PROJECTILE_SPRITE_BLUE_DISK, PROJECTILE_SPRITE_PURPLE_DISK, PROJECTILE_SPRITE_BLUE_DISK, PROJECTILE_SPRITE_RED_DISK, PROJECTILE_SPRITE_WARM_DISK},
                       0, -15, prng_ranged_integer(10, 40),
                       25, 75, 80, 0.5
                    );
                    boss1_state.next_think_action_t = current_t + prng_ranged_float(4.5, 6.5);
                 end

                 if action_rng == 2 or action_rng == 8 then
                    Boss1_Sprout1(
                     boss1_state.last_good_position,
                     16,
                        {PROJECTILE_SPRITE_GREEN_DISK, PROJECTILE_SPRITE_CAUSTIC_DISK, PROJECTILE_SPRITE_GREEN_DISK, PROJECTILE_SPRITE_HOT_PINK_DISK, PROJECTILE_SPRITE_WARM_DISK},
                        prng_ranged_integer(4, 9), -30, prng_ranged_integer(10, 120),
                        10, 65, 70, 0.23
                    );
                    boss1_state.next_think_action_t = current_t + prng_ranged_float(4.0, 4.5);
                 end

                 if action_rng == 3 then
                    if current_t >= boss1_state.next_grid_action_t  then
                       Boss1_GridCrossBoxIn(3.0, 2, 4, PROJECTILE_SPRITE_GREEN, PROJECTILE_SPRITE_GREEN);
                       boss1_state.next_grid_action_t = current_t + prng_ranged_float(10.0, 12.0);
                    end
                    boss1_state.next_think_action_t = current_t + prng_ranged_float(1.5, 3.0);
                 end

                 if action_rng == 4 then
                    if current_t >= boss1_state.next_grid_action_t  then
                       Boss1_GridCrossBoxIn(3.0, 3, 3, PROJECTILE_SPRITE_WRM_ELECTRIC, PROJECTILE_SPRITE_WRM_ELECTRIC);
                       boss1_state.next_grid_action_t = current_t + prng_ranged_float(10.0, 12.0);
                    end
                    boss1_state.next_think_action_t = current_t + prng_ranged_float(1.5, 3.0);
                 end

                 if action_rng == 5 or action_rng == 9 then
                    local sgn = 1;

                    if player_position_y() < boss1_state.last_good_position[2] then
                       sgn = -1;
                    end

                    Boss1_XCross_RainDown(
                       boss1_state.last_good_position,
                       prng_ranged_integer(6, 15),
                       PROJECTILE_SPRITE_WRM,
                       PROJECTILE_SPRITE_WRM_DISK,
                       55, -- present speed
                       sgn * 100, -- fire speed
                       sgn * 85,  -- acceleration
                       5, -- trailcount
                       1.0 -- time until chase
                    );
                    boss1_state.next_think_action_t = current_t + prng_ranged_float(2.5, 4.5);
                 end

                 if action_rng == 6 or action_rng == 10 then
                    if not boss1_raining then
                       Boss1_XCross_Chasing(
                          prng_ranged_integer(4, 6),
                          PROJECTILE_SPRITE_CAUSTIC,
                          PROJECTILE_SPRITE_CAUSTIC_DISK,
                          80, -- present speed
                          100, -- fire speed
                          20,  -- acceleration
                          6, -- trailcount
                          0.6, -- time until chase
                          1.15 -- chase for
                       )
                       boss1_state.next_think_action_t = current_t + prng_ranged_float(4.5, 6.5);
                    end
                 end

                 if action_rng == 7 then -- check for grid
                    if not boss1_raining then
                       Boss1_ExplosionChase(6);
                       boss1_state.next_think_action_t = current_t + prng_ranged_float(5.0, 6.5);
                    end
                 end
              end
           end
        end

        if enemy_valid(eid) then
           boss1_state.last_good_position = enemy_final_position(eid);
        end
        t_yield();
    end

    end_black_fade();
    -- we have latency of 1 dt after death :/
    local death_pattern = {
        PROJECTILE_SPRITE_CAUSTIC_DISK, 
        PROJECTILE_SPRITE_BLUE_DISK,
        PROJECTILE_SPRITE_RED_DISK,
        PROJECTILE_SPRITE_PURPLE_DISK
    };

    enable_grazing();
    Boss1_Sprout1(
      boss1_state.last_good_position,
      45,
        death_pattern,
        0, 15, 5,
        5, 75, 200, 0.28
    );

    if enable_boss_death_explosion then
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
    end
    -- Wait until minions have died.
    while remaining_hexbind_minions() do 
        t_yield();
    end
    t_wait(1.5);
    t_complete_stage();
end

-- Movement scheduler
function _Stage1_Boss_Movement_Logic(eid)
   t_wait(2.5); -- wait out invincibiility.
   boss1_state.last_good_position = enemy_final_position(eid);
   while enemy_valid(eid) do 
       local boss_health_percentage = enemy_hp_percent(boss1_state.me);
       boss1_state.last_good_position = enemy_final_position(eid);

       -- Handle Boss Movement
       if boss1_state.wants_to_move then
          if boss1_state.moving_to == false then
             boss1_state.moving_to = true;

             if boss1_state.move_method == 'teleport' then
                enemy_task_lambda(
                   boss1_state.me,
                   function(e)
                      explosion_hazard_new(
                         boss1_state.last_good_position[1],
                         boss1_state.last_good_position[2],
                         35, 0.01, 0.01);
                      explosion_hazard_new(
                         boss1_state.move_target[1],
                         boss1_state.move_target[2],
                         35, 0.01, 0.01);
                      t_wait(1);
                      if enemy_valid(eid) then
                         enemy_set_position(
                            e,
                            boss1_state.move_target[1],
                            boss1_state.move_target[2]
                         );
                         boss1_state.last_good_position = enemy_final_position(eid);
                         boss1_state.wants_to_move = false;
                         boss1_state.moving_to = false;
                      end
                   end
                );
             end

             -- ?
             if boss1_state.move_method == 'slide' then
             end
          end
       end

       t_yield();
   end
end

-- Some sub logic related to boss's secondary tasks
function _Stage1_Boss_Maintain_Hexes_Logic(eid)
   t_wait(2.5); -- wait out invincibiility.
   boss1_state.last_good_position = enemy_final_position(eid);
   while enemy_valid(eid) do 
       local boss_health_percentage = enemy_hp_percent(boss1_state.me);
       boss1_state.last_good_position = enemy_final_position(eid);

       -- NOTE(jerry): needs to be put here due to frame latency reasons.
       if boss1_state.phase == 1 or boss1_state.phase == 3 or boss1_state.phase == 5 then
          if not remaining_hexbind_minions() then
             boss1_state.phase = boss1_state.phase + 1;
             enemy_end_invincibility(boss1_state.me);
             enemy_hurt(boss1_state.me, BOSS_HP_LOSS_FROM_HEX_FINISH);
             Boss1_End_Hex();
             award_points(50000);
             player_add_life();
             play_sound(load_sound('res/snds/hitcrit.wav'))
             explosion_hazard_new(
                boss1_state.last_good_position[1],
                boss1_state.last_good_position[2],
                95, 0.00, 0.00);
            do
               local particle_emitter_ptr = enemy_get_particle_emitter(boss1_state.me, 0);
               particle_emitter_set_active(particle_emitter_ptr, false);
            end
          else
             -- invincible while hexbinders are active
            Boss1_Enemy_HexShield(boss1_state.me);
            enemy_begin_invincibility(boss1_state.me, true, 99999);
          end
       end

       -- Spawn Hex Binders
       -- Is there a more elegant way to write the code?
       -- probably, right now that doesn't matter!
       if boss_health_percentage <= 0.75 and boss1_state.phase == 0 then
          boss1_state.phase = 1;
          Boss1_Begin_Hex();
          Game_Spawn_Stage1_Boss_HexBind0();
          Game_Spawn_Stage1_Boss_HexBind1();
          enemy_begin_invincibility(boss1_state.me, true, 99999);
       end

       if boss_health_percentage <= 0.50 and boss1_state.phase == 2 then
          boss1_state.phase = 3;
          Boss1_Begin_Hex();
          Game_Spawn_Stage1_Boss_HexBind0();
          Game_Spawn_Stage1_Boss_HexBind1();
          enemy_begin_invincibility(boss1_state.me, true, 99999);
       end

       if boss_health_percentage <= 0.27 and boss1_state.phase == 4 then
          boss1_state.phase = 5;
          Boss1_Begin_Hex();
          Game_Spawn_Stage1_Boss_HexBind0();
          Game_Spawn_Stage1_Boss_HexBind1();
          Game_Spawn_Stage1_Boss_HexBind2();
          Game_Spawn_Stage1_Boss_HexBind3();
          enemy_begin_invincibility(boss1_state.me, true, 99999);
       end
       t_yield();
   end
end

-- Some sub logic related to boss's secondary tasks
function _Stage1_Boss_ImmunityToBurstLaser_Logic(eid)
   t_wait(2.5); -- wait out invincibiility.
   boss1_state.last_good_position = enemy_final_position(eid);
   while enemy_valid(eid) do 
       local boss_health_percentage = enemy_hp_percent(boss1_state.me);
       boss1_state.last_good_position = enemy_final_position(eid);

       if (player_using_burst_power1()) then
          enemy_begin_invincibility(boss1_state.me, true, 99999);
       else
          -- there's a small window where the boss can get hit
          -- but that's okay, since you're very vulnerable after burstpower1
          -- anyway...
          t_wait(0.65);
          enemy_end_invincibility(boss1_state.me);
       end
       t_yield();
   end
end

function _Stage1_Boss_HexBind(eid, state)
   -- Tell the boss I exist.
   if state.id == 0 then
      boss1_state.hexbind0 = eid;
   end
   if state.id == 1 then
      boss1_state.hexbind1 = eid;
   end
   if state.id == 2 then
      boss1_state.hexbind2 = eid;
   end
   if state.id == 3 then
      boss1_state.hexbind3 = eid;
   end

   while enemy_valid(eid) do
      if enemy_valid(boss1_state.me) then
         Boss1_Enemy_HexAttractor(eid, boss1_state.me);
      end

      -- next think action depends on what attack was chosen
      local current_t = enemy_time_since_spawn(eid);
      local position = enemy_final_position(eid);
      if current_t >= state.next_think_action_t then
         local action_rng = prng_ranged_integer(0, 100);
         -- 40% of attack
         local attack_chance = 40;
         if boss1_state.phase == 5 then
            attack_chance = 20;
         end

         if action_rng < attack_chance then
            action_rng = prng_ranged_integer(0, 2);

            if action_rng == 0  then
               Boss1_Sprout1(
                  position,
                  4,
                  {PROJECTILE_SPRITE_BLUE_DISK, PROJECTILE_SPRITE_PURPLE_DISK, PROJECTILE_SPRITE_BLUE_DISK, PROJECTILE_SPRITE_RED_DISK, PROJECTILE_SPRITE_WARM_DISK},
                  0, -15, prng_ranged_integer(10, 20),
                  25, 35, 40, 0.5
               );
               state.next_think_action_t = current_t + prng_ranged_float(4.5, 7.5);
            end

            if action_rng == 1 then
               Boss1_Sprout1(
                  position,
                  6,
                  {PROJECTILE_SPRITE_GREEN_DISK, PROJECTILE_SPRITE_CAUSTIC_DISK, PROJECTILE_SPRITE_GREEN_DISK, PROJECTILE_SPRITE_HOT_PINK_DISK, PROJECTILE_SPRITE_WARM_DISK},
                  prng_ranged_integer(4, 6), -30, prng_ranged_integer(10, 50),
                  25, 35, 40, 0.23
               );
               state.next_think_action_t = current_t + prng_ranged_float(3.5, 5.5);
            end

            if action_rng == 2 then
               local sgn = 1;

               if player_position_y() < boss1_state.last_good_position[2] then
                  sgn = -1;
               end

               Boss1_XCross_RainDown(
                  position,
                  prng_ranged_integer(6, 15),
                  PROJECTILE_SPRITE_WRM,
                  PROJECTILE_SPRITE_WRM_DISK,
                  55, -- present speed
                  sgn * 50, -- fire speed
                  sgn * 45,  -- acceleration
                  3, -- trailcount
                  1.0 -- time until chase
               );
               state.next_think_action_t = current_t + prng_ranged_float(2.5, 4.5);
            end
         else
            state.next_think_action_t = current_t + prng_ranged_float(2.5, 4.5);
         end
      end

      t_yield();
   end
   state.next_think_action_t = -9999;
end

function _Stage1_Boss_MusicPlayer(e)
   while enemy_valid(e) do
      if (music_playing() == false) then
         play_music(track1);
      end
      t_yield();
   end
end

function Game_Spawn_Stage1_Boss()
   local e = enemy_new();
   local initial_boss_pos = v2(play_area_width()/2, 50);
   enemy_set_hp(e, BOSS_HP); -- TODO for now
   enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
   enemy_set_visual(e, ENTITY_SPRITE_BOSS1);
   enemy_show_boss_hp(e, "WITCH");
   -- The boss takes "three threads" of logic.
   async_task_lambda(_Stage1_Boss_Logic, e);
   async_task_lambda(_Stage1_Boss_Movement_Logic, e);
   async_task_lambda(_Stage1_Boss_Maintain_Hexes_Logic, e);
   async_task_lambda(_Stage1_Boss_ImmunityToBurstLaser_Logic, e);
   async_task_lambda(_Stage1_Boss_MusicPlayer, e);
   boss1_state.me = e;
   boss1_state.starting_position = initial_boss_pos;

   do
      -- Some move positions, should make bullet shooting more interesting
      local i = 0;
      boss1_teleport_positions[i] = initial_boss_pos;
      i=i+1;
      boss1_teleport_positions[i] = initial_boss_pos;
      i=i+1;
      boss1_teleport_positions[i] = initial_boss_pos;
      i=i+1;
      boss1_teleport_positions[i] = v2(initial_boss_pos[1] + 70, initial_boss_pos[2] + 30);
      i=i+1;
      boss1_teleport_positions[i] = v2(initial_boss_pos[1] - 70, initial_boss_pos[2] + 30);
      i=i+1;
      boss1_teleport_positions[i] = v2(initial_boss_pos[1] + 180, initial_boss_pos[2] + 50);
      i=i+1;
      boss1_teleport_positions[i] = v2(initial_boss_pos[1] - 150, initial_boss_pos[2] + 80);
      i=i+1;
      boss1_teleport_positions[i] = v2(initial_boss_pos[1], play_area_height()/2);
      i=i+1;
      boss1_teleport_positions[i] = v2(initial_boss_pos[1] - 70, play_area_height()/2);
      i=i+1;
      boss1_teleport_positions[i] = v2(initial_boss_pos[1] + 70, play_area_height()/2);
      i=i+1;
      boss1_teleport_positions[i] = v2(initial_boss_pos[1], play_area_height()-30);
      i=i+1;
      boss1_teleport_positions[i] = v2(initial_boss_pos[1]-40, play_area_height()-30);
      i=i+1;
      boss1_teleport_positions[i] = v2(initial_boss_pos[1]+40, play_area_height()-30);
      i=i+1;
   end;
   return e;
end

function Game_Spawn_Stage_Boss_HexBinder(where, state)
   local e = enemy_new();
   local initial_boss_pos = where;
   enemy_set_hp(e, MINI_BOSS_HP);
   enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
   enemy_set_visual(e, ENTITY_SPRITE_HEXBINDER);
   enemy_show_boss_hp(e, "HEX BINDING");
   enemy_begin_invincibility(e, true, 2.5);
   async_task_lambda(_Stage1_Boss_HexBind, e, state);
   return e;
end

function Game_Spawn_Stage1_Boss_HexBind0()
   local initial_boss_pos = v2(play_area_width()/2 + 80, 70);
   local e = Game_Spawn_Stage_Boss_HexBinder(
      initial_boss_pos, hexbind0_state
   );
   return e;
 end
 
 function Game_Spawn_Stage1_Boss_HexBind1()
    local initial_boss_pos = v2(play_area_width()/2 - 80, 70);
    local e = Game_Spawn_Stage_Boss_HexBinder(
      initial_boss_pos, hexbind1_state
    );
    return e;
 end

function Game_Spawn_Stage1_Boss_HexBind2()
    local initial_boss_pos = v2(play_area_width()/2 + 80, 40);
    local e = Game_Spawn_Stage_Boss_HexBinder(
      initial_boss_pos, hexbind2_state
    );
    return e;
 end
 
 function Game_Spawn_Stage1_Boss_HexBind3()
    local initial_boss_pos = v2(play_area_width()/2 - 80, 40);
    local e = Game_Spawn_Stage_Boss_HexBinder(
      initial_boss_pos, hexbind3_state
    );
    return e;
 end
