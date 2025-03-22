engine_dofile("stages/constants.lua")
engine_dofile("stages/utility.lua")

function spawn_bullet_line(center, how_many, spacing, scale, direction, speed, src)
   local new_bullets = {};
   local direction     = v2_normalized(direction);
   local pattern_width = spacing * how_many + scale[1] * how_many;
   local perpendicular_direction = v2_perp(direction);

   -- No +=/-= hurts :/
   center[1] = center[1] - (perpendicular_direction[1] * (pattern_width/2));
   center[2] = center[2] - (perpendicular_direction[2] * (pattern_width/2));

   bullet_i = 1;

   for i=1, how_many do
      local nb = bullet_new(src);
      new_bullets[bullet_i] = nb;
      bullet_set_position(nb, center[1], center[2]);
      bullet_set_velocity(nb, direction[1] * speed, direction[2] * speed);
      bullet_set_scale(nb, scale[1], scale[2])
      bullet_i = bullet_i + 1;

      center[1] = center[1] + perpendicular_direction[1] * (spacing + scale[1])
      center[2] = center[2] + perpendicular_direction[2] * (spacing + scale[1])
   end

   return new_bullets;
end

function spawn_bullet_arc_pattern2(center, how_many, arc_degrees, direction, speed, distance_from_center, src)
   local new_bullets = {};
   local arc_sub_length = arc_degrees / how_many;
   local direction_angle = dir_to_angle(direction);
   
   local bullet_i = 1;
   
   for i=0, how_many do
      local angle = direction_angle + arc_sub_length * (i - (how_many/2));
      local current_arc_direction = v2_direction_from_degree(angle);
      local position = v2_add(center, v2(current_arc_direction[1] * distance_from_center, current_arc_direction[2] * distance_from_center));
      
      local nb = bullet_new(src);
      new_bullets[bullet_i] = nb;
      bullet_set_position(nb, position[1], position[2]);
      bullet_set_scale(nb, 10, 10);
      bullet_set_velocity(nb, current_arc_direction[1] * speed, current_arc_direction[2] * speed);
      bullet_set_lifetime(nb, 10);
      bullet_i = bullet_i + 1;
   end
   return new_bullets;
end

-----------------
--
-- Some really generic bullet patterns if I got nothing else to try...
--
-- these are "staticy" patterns,
-- some enemies have these patterns built-in and I'm not refactoring anything
-- this late in development cause I just wanna finish the game!
--
-- So stuff might suffer, but yeah...
--
-----------------
function Bullet_Pattern_Sprout_Outwards360(
   center, 
   bvisual, 
   bspeed, 
   angle_step,
   angoffset,
   angstart,
   angend,
   blifetime, 
   bscale, 
   bacceleration,
   btrailcount,
   btrailrecordspeed
)
   angoffset = angoffset or 0;
   angstart = angstart or 1;
   angend = angend or 360;
   blifetime = blifetime or 15;
   bscale = bscale or 5;
   bacceleration = bacceleration or 0;
   btrailcount = btrailcount or 0;
   btrailrecordspeed = btrailrecordspeed or 0;

   local mvsgn = 1;
   if angle_step < 0 then
      local temp = angend;
      angend = angstart;
      angstart = temp;
      mvsgn = -1;
   end

   -- print(angstart, angend, angle_step)

   for angle=angstart,angend,angle_step do
      local bullet = bullet_new(BULLET_SOURCE_ENEMY);
      bullet_set_position(bullet, center[1], center[2]);
      bullet_set_visual(bullet, bvisual)
      bullet_set_lifetime(bullet, blifetime);
      bullet_set_scale(bullet, bscale,bscale);
      bullet_set_visual_scale(bullet, bscale/10, bscale/10);

      local bdir = v2_direction_from_degree(angle+angoffset);
      bullet_set_velocity(bullet, mvsgn*bdir[1] * bspeed, mvsgn*bdir[2] * bspeed);
      bullet_set_acceleration(bullet, mvsgn*bdir[1] * bacceleration, mvsgn*bdir[2] * bacceleration);
      bullet_start_trail(bullet, btrailcount);
      bullet_set_trail_record_speed(btrailrecordspeed);
   end
end

------------------
-- Game preset enemy types because I realized programming enemies individually is dumb.
-- name scheme
--
-- Make_Enemy_Description_{Variation_Stage_Level}

-- NOTE: variation is for behavior. Visually enemies are skinned manually.
------------------

-- Some enemy prototypes here
-- NOTE: Please... avoid touching these because editting them a lot might
-- significantly break older levels...

--
-- Linearly Moves to a position
-- and fires a basic burst. Allows extra bursts which
-- the bursts themselves move linearly. These are a basic popcorn.
-- will rotate slowly
--
-- The bursts are designed to look like a spiral
-- Will for in a 360 degree radius.
--
-- made for 1_2
-- uses normal "bigger" projectiles
--
-- TODO, projectile parameterization?
-- might be easier to just copy and paste for the sake of the game tbh.
--
--
function Make_Enemy_Burst360_1_1_2(
      hp,
      initial_position,
      target_position,
      target_position_lerp_t,

      fire_delay,
      burst_count,

      angle_spacing,
      displacement_shift,

      bullet_present_velocity,
      bullet_velocity,

      exit_direction,
      exit_velocity,
      exit_acceleration,

      bullet_visual,
      tcount
   )
   local e = enemy_new();
   enemy_set_hp(e, hp);
   enemy_set_position(e, initial_position[1], initial_position[2]);
   tcount = tcount or 0;
   exit_direction = v2_normalized(exit_direction);

   enemy_task_lambda(
      e,
      function(e)
         -- cannot use asymptopic movement yet
         enemy_linear_move_to(e, target_position[1], target_position[2], target_position_lerp_t);
         t_wait(fire_delay);
         for displacement=0,burst_count do
            for angle=1,360,angle_spacing do
               local current_arc_direction = v2_direction_from_degree((angle + displacement*displacement_shift) % 360);
               local b = bullet_make(
                  BULLET_SOURCE_ENEMY,
                  enemy_position(e),
                  bullet_visual,
                  v2(0.5, 0.5),
                  v2(5, 5)
               )
               bullet_start_trail(b, tcount);
               bullet_set_trail_modulation(b, 0.8,0.8,0.8,0.3);
               bullet_task_lambda(
                  b,
                  function(b)
                     bullet_set_velocity(b, current_arc_direction[1] * bullet_present_velocity, current_arc_direction[2] * bullet_present_velocity);
                     t_wait(clamp(0.10 * displacement, 0.1, 0.5));
                     bullet_reset_movement(b);
                     t_wait(0.5);
                     bullet_set_velocity(b, current_arc_direction[1] * bullet_velocity, current_arc_direction[2] * bullet_velocity);
                  end
               )
               -- bullet_set_velocity(b, current_arc_direction[1] * 30, current_arc_direction[2] * 30);
               bullet_set_lifetime(b, 15);
            end
            play_sound(random_attack_sound());
            t_wait(0.37);
         end

         t_wait(0.75);

         enemy_set_velocity(e, exit_direction[1] * exit_velocity, exit_direction[2] * exit_velocity);
         enemy_set_acceleration(e, exit_direction[1] * exit_acceleration, exit_direction[2] * exit_acceleration);
      end
   );

   return e;
end

-- Same as above with a spin.
function Make_Enemy_Burst360_1_1_2_EX(
      hp,
      initial_position,
      target_position,
      target_position_lerp_t,

      fire_delay,
      burst_count,

      angle_spacing,
      displacement_shift,

      bullet_present_velocity,
      bullet_velocity,

      exit_direction,
      exit_velocity,
      exit_acceleration,

      angle_displacement_per_frame,

      bullet_visual,
      tcount
   )
   local e = enemy_new();
   enemy_set_hp(e, hp);
   enemy_set_position(e, initial_position[1], initial_position[2]);
   tcount = tcount or 0;
   exit_direction = v2_normalized(exit_direction);

   enemy_task_lambda(
      e,
      function(e)
         -- cannot use asymptopic movement yet
         enemy_linear_move_to(e, target_position[1], target_position[2], target_position_lerp_t);
         t_wait(fire_delay);
         for displacement=0,burst_count do
            for angle=1,360,angle_spacing do
               local current_arc_direction = v2_direction_from_degree((angle + displacement*displacement_shift) % 360);
               local b = bullet_make(
                  BULLET_SOURCE_ENEMY,
                  enemy_position(e),
                  bullet_visual,
                  v2(0.5, 0.5),
                  v2(5, 5)
               )
               bullet_start_trail(b, tcount);
               bullet_set_trail_modulation(b, 0.8,0.8,0.8,0.3);
               bullet_task_lambda(
                  b,
                  function(b)
                     local sang = (angle + displacement*displacement_shift) % 360;
                     bullet_set_velocity(b, current_arc_direction[1] * bullet_present_velocity, current_arc_direction[2] * bullet_present_velocity);
                     t_wait(clamp(0.10 * displacement, 0.1, 0.5));
                     bullet_reset_movement(b);
                     t_wait(0.5);
                     while true do
                        local current_arc_direction = v2_direction_from_degree(sang);
                        bullet_set_velocity(b, current_arc_direction[1] * bullet_velocity, current_arc_direction[2] * bullet_velocity);
                        sang = sang + angle_displacement_per_frame;
                        t_yield();
                     end
                  end
               )
               -- bullet_set_velocity(b, current_arc_direction[1] * 30, current_arc_direction[2] * 30);
               bullet_set_lifetime(b, 15);
            end
            play_sound(random_attack_sound());
            t_wait(0.37);
         end

         t_wait(0.75);

         enemy_set_velocity(e, exit_direction[1] * exit_velocity, exit_direction[2] * exit_velocity);
         enemy_set_acceleration(e, exit_direction[1] * exit_acceleration, exit_direction[2] * exit_acceleration);
      end
   );

   return e;
end

-- I need to take advantage of these behavior things way more.
function Enemy_AddExplodeOnDeathBehavior(e, explosion_radius, warning_timer, explosion_timer)
   enemy_task_lambda(
       e,
      function(e)
         local position = enemy_final_position(e);
         local oob      = false;
         while enemy_valid(e) do
            if enemy_valid(e) then
               position = enemy_final_position(e);
               oob      = out_of_bounds(position);
            end
            t_yield()
         end

         if not oob then
            explosion_hazard_new(position[1], position[2], explosion_radius, warning_timer, explosion_timer);
         end
      end
   );
end

function Enemy_AddSpinBehavior(e, angle_rotation_per_frame, radiusx, radiusy)
   enemy_task_lambda(
      e,
      function (e)
         local angle = 0;
         while enemy_valid(e) do
            -- enemy_set_relative_position(e, 1, 2);
            enemy_set_relative_position(e, math.cos(math.rad(angle)) * radiusx, math.sin(math.rad(angle)) * radiusy);
            angle = angle + angle_rotation_per_frame;
            t_yield();
         end
      end,
      radiusx,
      radiusy
   )
end

--
--
-- Will spawn a sprinkle of really dumb pop corn enemies that move linearly
-- 
-- They will look semi random.
--
--

-- NOTE: due to how these guys are made I might need to skin these guys *manually* for each
-- permutation of the function, which is fine since only a select few enemy variations are popcorns.
function Make_BrainDead_Enemy_Popcorn1(
      amount,
      start_position,
      spawn_delay,
      per_hp,
      hspeed,
      vspeed,
      x_vary,
      h_vary,
      sign_modv,
      sign_modh,
      sprite)
   local enemies = {};
   local sprite = sprite or ENTITY_SPRITE_BAT_A;
   for i=1, amount do
      local e = enemy_new();
      enemy_set_visual(e, sprite);
      local vsgn = 1;
      local hsgn = 1;
      if sign_modv ~= -1 then
         if (i % sign_modv == 0) then
            vsgn = -1;
         end
      end

      if sign_modh ~= -1 then
         if (i % sign_modh == 0) then
            hsgn = -1;
         end
      end

      enemy_set_hp(e, per_hp);
      enemy_set_position(e,
                         start_position[1] + math.sin(i*10) * x_vary,
                         start_position[2] + math.sin(i*10) * h_vary);
      enemy_set_velocity(e, hspeed * hsgn, vspeed * vsgn);
      t_wait(spawn_delay);

      enemies[i] = e;
   end

   return enemies;
end

-- move forward with a fixed delay.
-- then start homing with a trail.
-- stop homing after x seconds
function Bullet_Add_HomingBehavior_1_1(
      b,
      travel_direction,
      travel_velocity,
      travel_time,
      delay_before_homing,
      homing_time,
      homing_velocity,
      homing_acceleration,
      visual_prehoming,
      visual_posthoming,
      use_trail)
   bullet_task_lambda(
      b,
      function(b)
         bullet_set_visual(b, visual_prehoming)
         bullet_set_visual_scale(b, 0.5, 0.5);
         bullet_move_linear(b, travel_direction, 80);
         t_wait(travel_time);
         bullet_reset_movement(b);
         t_wait(delay_before_homing);
         bullet_set_visual(b, visual_posthoming);
         if (use_trail ~= -1) then
            bullet_start_trail(b, use_trail);
         end
         bullet_set_trail_modulation(b, 1, 1, 1, 0.3);

         local spawn_time = bullet_time_since_spawn(b);
         while true do
            local dt = bullet_time_since_spawn(b) - spawn_time;
            local aim = dir_to_player(bullet_position(b));

            aim[1] = aim[1] * (homing_velocity + (dt * homing_acceleration));
            aim[2] = aim[2] * (homing_velocity + (dt * homing_acceleration));

            bullet_set_velocity(b, aim[1], aim[2]);

            if dt > homing_time then
               break;
            end

            t_yield();
         end
      end
   );
end

--function spawn_bullet_arc_pattern2(center, how_many, arc_degrees, direction, speed, distance_from_center, src)
-- basic arc shooter similar to burst360_1_1_2
function Make_Enemy_ArcPattern2_1_1_2(
      hp,
      initial_position,
      target_position,
      target_position_lerp_t,

      fire_delay,
      burst_count,

      arc_degrees,
      bullets_per_burst,
      bullet_velocity,
      bullet_distance_from_center,

      exit_direction,
      exit_velocity,
      exit_acceleration,

      bullet_visual
   )

   local e = enemy_new();
   enemy_set_hp(e, hp);
   enemy_set_position(e, initial_position[1], initial_position[2]);

   exit_direction = v2_normalized(exit_direction);

   enemy_task_lambda(
      e,
      function(e)
         -- cannot use asymptopic movement yet
         enemy_linear_move_to(e, target_position[1], target_position[2], target_position_lerp_t);
         t_wait(fire_delay);
         for displacement=0,burst_count do
            local bullets = spawn_bullet_arc_pattern2(enemy_position(e), bullets_per_burst, arc_degrees, v2_normalized(bullet_velocity), v2_magnitude(bullet_velocity), bullet_distance_from_center, BULLET_SOURCE_ENEMY);
            for i,b in ipairs(bullets) do
               bullet_set_visual(b, bullet_visual)
               bullet_set_visual_scale(b, 0.5, 0.5);
            end
            play_sound(random_attack_sound());
            t_wait(0.37);
         end

         t_wait(0.75);
         enemy_set_velocity(e, exit_direction[1] * exit_velocity, exit_direction[2] * exit_velocity);
         enemy_set_acceleration(e, exit_direction[1] * exit_acceleration, exit_direction[2] * exit_acceleration);
      end
   );

   return e;
end

-- Linearly travels across the screen,
-- while spinning and firing "star/sakura" 
function Make_Enemy_Spinner_1_1_2(hp,
                                  initial_position,
                                  direction,
                                  velocity,
                                  fire_delay_per_burst,
                                  initial_delay,
                                  bspeed,
                                  radiusx,
                                  radiusy,
                                  bullet_visual,
                                  trail
                                 )
   local e = enemy_new();
   local trailcount = trail or 0;
   enemy_set_hp(e, hp);
   enemy_set_position(e, initial_position[1], initial_position[2]);

   enemy_move_linear(e, direction, velocity);

   Enemy_AddSpinBehavior(e, 2, radiusx, radiusy);
   -- enemy_task_lambda(
   --    e,
   --    function (e)
   --       local angle = 0;
   --       while enemy_valid(e) do
   --          -- enemy_set_relative_position(e, 1, 2);
   --          enemy_set_relative_position(e, math.cos(math.rad(angle)) * radiusx, math.sin(math.rad(angle)) * radiusy);
   --          angle = angle + 2;
   --          t_yield();
   --       end
   --    end,
   --    radiusx,
   --    radiusy
   -- )
   enemy_task_lambda(
      e,
      function (e, bspeed)
         t_wait(initial_delay);
         local displacement = 10;
         while enemy_valid(e) do
            for angle=1,360,30 do
               local eposition = enemy_final_position(e);
               local bullet = bullet_new(BULLET_SOURCE_ENEMY);
               bullet_set_position(bullet, eposition[1], eposition[2]);
               bullet_set_visual(bullet, bullet_visual)
               bullet_set_lifetime(bullet, 15);
               bullet_set_scale(bullet, 5, 5);
               bullet_set_visual_scale(bullet, 0.5, 0.5);

               local bdir = v2_direction_from_degree(angle + displacement);
               bullet_set_velocity(bullet, bdir[1] * bspeed, bdir[2] * bspeed);
               bullet_start_trail(bullet, trailcount);
               displacement = displacement + 25;
            end
            play_sound(random_attack_sound());
            t_wait(fire_delay_per_burst);
         end
      end,
      bspeed
   )
   return e;
end

-- Enemy that spawns in a place, does a spin dodge trap
-- linear.
-- Will exit after duration is over.
-- These are very specific behaviors tbh.
function Make_Enemy_SpinTrip_2_1_1(hp,
                                   position,
                                   spoke_arc,
                                   spoke_per_frame_angular_velocity,
                                   exit_direction,
                                   exit_speed,
                                   exit_acceleration,
                                   duration,
                                   bullet_visual,
                                   trailcount
                                  )
   local e = enemy_new();
   local tcount = trailcount or 0;
   enemy_set_hp(e, hp);
   enemy_set_position(e, position[1], position[2]);

   exit_direction = v2_normalized(exit_direction);

   enemy_task_lambda(
      e,
      function(e)
         local start_time = enemy_time_since_spawn(e);
         local arc_displacement = 0;

         while (enemy_time_since_spawn(e) - start_time < duration) do
            local bspeed    = 35;
            local eposition = enemy_final_position(e);

            for angle=1,360,spoke_arc do
               local bullet = bullet_new(BULLET_SOURCE_ENEMY);
               bullet_set_position(bullet, eposition[1], eposition[2]);
               bullet_set_visual(bullet, bullet_visual);
               bullet_set_lifetime(bullet, 15);
               bullet_set_scale(bullet, 3, 3);
               bullet_start_trail(bullet, tcount);
               bullet_set_trail_modulation(bullet, 0.8, 0.8, 0.8, 0.3);
               bullet_set_visual_scale(bullet, 0.3, 0.3);

               local bdir = v2_direction_from_degree(angle + arc_displacement);
               bullet_set_velocity(bullet, bdir[1] * bspeed, bdir[2] * bspeed);
            end

            arc_displacement = arc_displacement + spoke_per_frame_angular_velocity;
            t_wait(0.25);
         end

         enemy_move_linear(e, exit_direction, exit_speed);
         enemy_set_acceleration(e, exit_direction[1] * exit_acceleration, exit_direction[2] * exit_acceleration);
      end
   )

   return e;
end


-- Not really meant to be killable, but you can farm points off of them!
function DramaticExplosion_SpawnSpinnerObstacle1_2_1(x,
   y,
   enemy_visual,
   spoke_per_frame_angular_velocity,
   duration,
   bullet_visual,
   exit_direction,
   trailcount)
   local enemy_position = v2(x, y);
   explosion_hazard_new(enemy_position[1], enemy_position[2], 15, 0.05, 0.15);
   t_wait(1.25);
   local e = Make_Enemy_SpinTrip_2_1_1(9999,
   enemy_position,
   60,
   spoke_per_frame_angular_velocity,
   exit_direction,
   50,
   30,
   duration,
   bullet_visual,
   trailcount);

   enemy_set_visual(e, enemy_visual);
   enemy_set_visual_scale(e, 1.3, 1.3);
   return e;
end

function Make_Enemy_ShotgunSpreader_2_1_1(
   hp, x, y,
   shot_times, per_burst_volley_count, 
   bullet_visual, cooldown, flee_cooldown, dir, speed,
   bullet_trail_count
)
   local e = enemy_new();
   enemy_set_scale(e, 10, 10);
   enemy_set_hp(e, hp);
   enemy_set_position(e, x, y);

   enemy_task_lambda(e,
      function (e)
         local trail_count = bullet_trail_count or 0;
         for burst=1, shot_times do
            local bullets = spawn_bullet_arc_pattern2(v2(x,y), per_burst_volley_count, 45, dir, speed, 8, BULLET_SOURCE_ENEMY);
            for i,b in ipairs(bullets) do
               bullet_set_visual(b, bullet_visual)
               bullet_set_visual_scale(b, 0.5, 0.5);
               bullet_set_scale(b, 5, 5)
               bullet_start_trail(b, trail_count);
            end
            t_wait(cooldown);
         end
         t_wait(flee_cooldown);
         enemy_set_acceleration(e, 0, 150);
      end
   );
   return e
end

function DramaticExplosion_SpawnShotgunSpread(
   x, y, 
   enemy_visual, hp, shot_times, per_burst_volley_count,
   bullet_visual, cooldown, flee_cooldown, dir, speed,
   bullet_trail_count)
   local enemy_position = v2(x, y);
   explosion_hazard_new(enemy_position[1], enemy_position[2], 15, 0.05, 0.15);
   t_wait(1.25);

   local e = Make_Enemy_ShotgunSpreader_2_1_1(
      hp,
      enemy_position[1], enemy_position[2],
      shot_times, per_burst_volley_count,
      bullet_visual,
      cooldown,
      flee_cooldown,
      dir,
      speed,
      bullet_trail_count
   );

   enemy_set_visual(e, enemy_visual);
   enemy_set_visual_scale(e, 1, 1);
   return e;
end

-- ORIGINALLY HARDCODED 4
-- ORIGINALLY HARDCODED PROJECTILE_SPRITE_GREEN
function _wave1_enemy_shoot_down_and_fly_to_side_1_1(e, x_down, y_down, t_mv_speed, side, max_bullets, projectile_visual, projectile_visual2)
   enemy_linear_move_to(e, enemy_position_x(e) + x_down, enemy_position_y(e) + y_down, t_mv_speed);
   local ex = enemy_position_x(e);
   local ey = enemy_position_y(e);
   for bullet=1, max_bullets do
      local b = bullet_make(BULLET_SOURCE_ENEMY, enemy_position(e), projectile_visual, v2(0.5, 0.5), v2(5, 5))
      Bullet_Add_HomingBehavior_1_1(
         b, V2_DOWN, 80, 1.54, 1, 0.375, 150, 180,
         projectile_visual, projectile_visual2,
         12
      );
      play_sound(random_attack_sound());
      t_wait(0.32);
   end
   t_wait(0.15);

   local sign = 1;
   if side == 0 then
      sign = -1;
   end

   enemy_set_acceleration(e, sign * 150, 125);
end

-- ORIGINALLY HARDCODED 10
-- ORIGINALLY HARD CODED PROJECTILE_SPRITE_HOT_PINK
function _wave1_enemy_shoot_down_and_fly_to_side2_1_1(e, x_down, y_down, t_mv_speed, side, max_bullets, projectile_visual)
   enemy_linear_move_to(e, enemy_position_x(e) + x_down, enemy_position_y(e) + y_down, t_mv_speed);
   local ex = enemy_position_x(e);
   local ey = enemy_position_y(e);
   for bullet=1, max_bullets do
      local b = bullet_new(BULLET_SOURCE_ENEMY);
      bullet_set_visual(b, projectile_visual);
      bullet_set_visual_scale(b, 0.25, 0.25);
      bullet_set_scale(b, 2, 2);
      bullet_set_position(b, enemy_position_x(e), enemy_position_y(e));
      local d  = v2_direction(v2(ex, ey), v2(play_area_width()/2, play_area_height()*0.75))
      bullet_set_velocity(b, d[1] * 75, d[2] * 75);
      play_sound(random_attack_sound());
      t_wait(0.30);
   end
   t_wait(0.15);

   local sign = 1;
   if side == 0 then
      sign = -1;
   end

   enemy_set_acceleration(e, sign * 150, 125);
end

-- ORIGINALLY HARDCODED 15
-- ORIGINALLY HARD CODED PROJECTILE_SPRITE_HOT_PINK
function _wave1_enemy_shoot_down_and_fly_to_side3_1_1(e, x_down, y_down, t_mv_speed, side, max_bullets, projectile_visual)
   enemy_linear_move_to(e, enemy_position_x(e) + x_down, enemy_position_y(e) + y_down, t_mv_speed);
   for bullet=1, max_bullets do
      local b = bullet_new(BULLET_SOURCE_ENEMY);
      bullet_set_visual(b, projectile_visual);
      bullet_set_visual_scale(b, 0.25, 0.25);
      bullet_set_scale(b, 2, 2);
      bullet_set_position(b, enemy_position_x(e), enemy_position_y(e));
      local ex = enemy_position_x(e);
      local ey = enemy_position_y(e);
      local d  = v2_direction_from_degree(enemy_time_since_spawn(e) * 16 + 100 * e);
      bullet_set_velocity(b, d[1] * 110, d[2] * 110);
      play_sound(random_attack_sound());
      t_wait(0.25);
   end
   t_wait(0.15);

   local sign = 1;
   if side == 0 then
      sign = -1;
   end

   enemy_set_acceleration(e, sign * 150, 125);
end

function Make_Enemy_Spinster_1_1(px, py,
                                 x_down, y_down,
                                 t_mv_speed,
                                 escape_side,
                                 wave_count,
                                 bullet_visual,
                                 angular_offset,
                                 b_angle_step, -- 10
                                 bspeed_base, -- 90
                                              -- 55
                                 bacel_base)
   local e = enemy_new();
   enemy_set_scale(e, 10, 10);
   enemy_set_position(e, px, py);
   enemy_set_hp(e, 2500); -- probably not killing this thing. That's okay.
   enemy_task_lambda(
      e,
      function(e)
         enemy_linear_move_to(e, enemy_position_x(e) + x_down, enemy_position_y(e) + y_down, t_mv_speed);
         local fire_delay = 0.10;
         local step       = b_angle_step;
         local bspeed     = bspeed_base;
         local bacel     =  bacel_base;

         for p=0,wave_count do
            if p == 1 then
               bspeed = bspeed * 1.5;
               bacel = bacel * 1.5;
               fire_delay = fire_delay * 0.5;
            end

            for angle=0, 180, step do
               local d = v2_direction_from_degree(angle+angular_offset);
               local dx = d[1];
               local dy = d[2];
               local b = bullet_new(BULLET_SOURCE_ENEMY);

               bullet_set_visual(b, bullet_visual);
               bullet_set_visual_scale(b, 0.5, 0.5);
               bullet_set_scale(b, 5, 5);
               bullet_set_position(b, enemy_position_x(e), enemy_position_y(e));
               bullet_set_velocity(b, dx * bspeed, dy * bspeed);
               bullet_set_acceleration(b, dx * bacel, dy * bacel);

               play_sound(random_attack_sound());
               t_wait(fire_delay);
            end
            for angle=180, 0, -step do
               local d = v2_direction_from_degree(angle+angular_offset);
               local dx = d[1];
               local dy = d[2];
               local b = bullet_new(BULLET_SOURCE_ENEMY);
               bullet_set_visual(b, bullet_visual);
               bullet_set_visual_scale(b, 0.5, 0.5);
               bullet_set_scale(b, 5, 5);
               bullet_set_position(b, enemy_position_x(e), enemy_position_y(e));
               bullet_set_velocity(b, dx * bspeed, dy * bspeed);
               bullet_set_acceleration(b, dx * bacel, dy * bacel);

               play_sound(random_attack_sound());
               t_wait(fire_delay);
            end
            t_wait(0.2);
         end
         t_wait(0.18);

         local sign = 1;
         if escape_side == 0 then
            sign = -1;
         end

         enemy_set_acceleration(e, sign * 150, 125);
      end
   )
   return e;
end

-- NOTE(jerry):
-- backport from Stage 1-1, which was written long before I had a better scripting API. It's been barely backported
-- correctly!
function Make_Enemy_SideMoverWave1_1_1(
      px, py,
      xdown, ydown,
      t_mvspeed,
      side, variation,
      max_bullets,
      visual1, visual2)
   local e = enemy_new();
   enemy_set_scale(e, 10, 10);
   enemy_set_hp(e, 27);
   enemy_set_position(e, px, py);
   if variation == 0 then
      enemy_set_task(e, "_wave1_enemy_shoot_down_and_fly_to_side_1_1", xdown, ydown, t_mvspeed, side, max_bullets, visual1, visual2);
   elseif variation == 1 then
      enemy_set_task(e, "_wave1_enemy_shoot_down_and_fly_to_side2_1_1", xdown, ydown, t_mvspeed, side, max_bullets, visual1, visual2);
   elseif variation == 2 then
      enemy_set_task(e, "_wave1_enemy_shoot_down_and_fly_to_side3_1_1", xdown, ydown, t_mvspeed, side, max_bullets, visual1, visual2);
   end
   return e;
end


-- This is a very bullet hell style attack
-- obviously it's pretty hard to dodge a lot of this, so I have to be pretty careful.
-- I've concluded it *is* possible to dodge but it's really hard, and I might need to reduce some visual noise.

-- RAIN STORM ATTACK: try to dodge
boss1_raining = false; -- NOTE(jerry): used to disable certain attacks that might be impossible to really dodge otherwise
function MainBoss1_RainCloud_Attack1(phase_cycle, duration, timebetweendroplets, cyclecount)
   local stop_task = 0;

   local spread = 40;
   local x_adv = play_area_width() / (spread - 5);
   timebetweendroplets = timebetweendroplets or 0.354;
   cyclecount = cyclecount or 30;

   async_task_lambda(
      function()
         disable_grazing();
         disable_bullet_to_points();
         start_black_fade(0.065);

         async_task_lambda(
            function()
               local cycle_point = phase_cycle;
               local gap_space   = 4;
               local ticks       = 1;
               while stop_task == 0 do
                  local gap_mid_x_range1 = normalized_sin(player_position_x() + phase_cycle * ticks) * spread + 3;
                  local gap_mid_x_range2 = normalized_sin(player_position_x() + phase_cycle/2 * ticks) * spread - 5;
                  local gap_mid_x_range3 = normalized_sin(player_position_x() + phase_cycle * ticks) * spread + 10;
                  local gap_mid_x_range4 = normalized_sin(player_position_x() + phase_cycle/4 * ticks) * spread - 3;
                  for i=1, cyclecount do
                     local variance_x = math.sin(i + phase_cycle*i + duration * i * ticks + player_position_x()) * 5;
                     local variance_y = math.sin(i + phase_cycle*i + duration * i * ticks + player_position_x()) * 5;
                     local variance_v = math.sin((i+4) + phase_cycle + duration/2 * i * ticks + player_position_x()) * 48;

                     if i > gap_mid_x_range1-gap_space and i < gap_mid_x_range1+gap_space or
                        i > gap_mid_x_range2-gap_space and i < gap_mid_x_range2+gap_space or
                        i > gap_mid_x_range3-gap_space and i < gap_mid_x_range3+gap_space or
                        i > gap_mid_x_range4-gap_space and i < gap_mid_x_range4+gap_space
                     then
                     else
                        local bullet = bullet_new(BULLET_SOURCE_ENEMY);
                        bullet_set_position(bullet, i * x_adv - 10 + variance_x, -10 + variance_y);
                        bullet_set_scale(bullet, 0.25, 0.25);
                        bullet_set_visual(bullet, PROJECTILE_SPRITE_BLUE_DISK);
                        bullet_set_visual_scale(bullet, 0.25, 0.25);
                        bullet_start_trail(bullet, 12);
                        bullet_set_trail_modulation(bullet, 0.8, 0.8, 0.8, 0.3);
                        bullet_set_velocity(bullet, 0, 180 + variance_v);
                     end
                  end
                  cycle_point = cycle_point + phase_cycle / 2;
                  ticks = ticks + 1;
                  t_wait(timebetweendroplets);
               end
            end
         )

         boss1_raining = true;
         t_wait(duration)
         stop_task = 1;
         enable_grazing();
         enable_bullet_to_points();
         t_wait(2.5);
         boss1_raining = false;
         end_black_fade();
      end
   )
end

function MainBoss1_RainCloud_Attack2(phase_cycle, duration, timebetweendroplets, cyclecount)
   local stop_task = 0;

   local spread = 35;
   local x_adv = play_area_width() / (spread);
   start_black_fade(0.065);
   timebetweendroplets = timebetweendroplets or 0.200;
   cyclecount = cyclecount or 25;

   async_task_lambda(
      function()
         disable_grazing();
         disable_bullet_to_points();
         disable_burst_charge();
         async_task_lambda(
            function()
               local cycle_point = phase_cycle;
               local gap_space   = 4;
               local ticks       = 1;
               while stop_task == 0 do
                  local gap_mid_x_range1 = normalized_sin(player_position_x() + phase_cycle * ticks) * spread + 3;
                  local gap_mid_x_range2 = normalized_sin(player_position_x() + phase_cycle/2 * ticks) * spread - 5;
                  local gap_mid_x_range3 = normalized_sin(player_position_x() + phase_cycle * ticks) * spread + 10;
                  local gap_mid_x_range4 = normalized_sin(player_position_x() + phase_cycle/4 * ticks) * spread - 3;
                  for i=1, cyclecount do
                     local variance_x = math.sin(i + phase_cycle*i + duration * i * ticks + player_position_x()) * 5;
                     local variance_y = math.sin(i + phase_cycle*i + duration * i * ticks + player_position_x()) * 15;
                     local variance_v = normalized_sin((i+4) + phase_cycle + duration/2 * i * ticks + player_position_x()) * 50;

                     if i > gap_mid_x_range1-gap_space and i < gap_mid_x_range1+gap_space or
                        i > gap_mid_x_range2-gap_space and i < gap_mid_x_range2+gap_space or
                        i > gap_mid_x_range3-gap_space and i < gap_mid_x_range3+gap_space or
                        i > gap_mid_x_range4-gap_space and i < gap_mid_x_range4+gap_space
                     then
                     else
                        local bullet = bullet_new(BULLET_SOURCE_ENEMY);
                        bullet_set_position(bullet, i * x_adv - 10 + variance_x, -10 + variance_y);
                        bullet_set_scale(bullet, 0.25, 0.25);
                        bullet_set_visual(bullet, PROJECTILE_SPRITE_RED_DISK);
                        bullet_set_visual_scale(bullet, 0.25, 0.25);
                        bullet_start_trail(bullet, 16);
                        bullet_set_trail_modulation(bullet, 0.8, 0.8, 0.8, 0.3);
                        bullet_set_velocity(bullet, 0, 225 - variance_v);
                        bullet_set_acceleration(bullet, 0, 5);
                     end
                  end
                  cycle_point = cycle_point + phase_cycle / 2;
                  ticks = ticks + 1;
                  t_wait(timebetweendroplets);
               end
            end
         )
         boss1_raining = true;
         t_wait(duration)
         enable_grazing();
         enable_burst_charge();
         enable_bullet_to_points();
         stop_task = 1;
         t_wait(2.5);
         boss1_raining = false;
         end_black_fade();
      end
   )
end

-- Chase the player with lasers to force them to move.
function LaserChaser_Horizontal_1_2(laser_count, max_wait_time)
   for i=1,laser_count do
      laser_hazard_new(player_position_y(), 10, 0, 0.05, 1);
      t_wait(max_wait_time - i*(max_wait_time/laser_count));
   end
end

function LaserChaser_Vertical_1_2(laser_count, max_wait_time)
   for i=1,laser_count do
      laser_hazard_new(player_position_x(), 10, 1, 0.05, 1);
      t_wait(max_wait_time - i*(max_wait_time/laser_count));
   end
end

-- Dramatic opening
-- TODO(jerry): adjust pattern?
function Stage1_Batflood()
   -- Bat flood
   -- free popcorn points
   local wave_count = 15;
   local per_wave   = 16;
   for i=1,wave_count do
      for j=1,per_wave do
         local e = enemy_new();
         enemy_set_hp(e, 5);
         enemy_set_position(e, j * (play_area_width() / (per_wave)) - 20, -30);
         enemy_set_visual(e, ENTITY_SPRITE_BAT_A);
         enemy_task_lambda(
            e,
            function (e)
               enemy_set_velocity(e, 0, 125);
               t_wait(1.0);
               if j > per_wave/2 then
                  enemy_set_acceleration(e, 50, 0);
               else
                  enemy_set_acceleration(e, -50, 0);
               end
            end
         )
      end
      t_wait(0.25);
   end
end

-- If you're lazy and don't want to do much for a level background
-- NOTE: expects the image to be 375, 480 or some ratio of it because that's the size of the playable region!.
-- Just for "playtesting" or "development"
function Generic_Infinite_Stage_ScrollV_Ref(image_resource_location, scrollspeed, offx, offy, layer)
   print("LUA REFERENCE IMPLEMENTATION")
   async_task_lambda(
      function ()
         local background_image         = load_image(image_resource_location);
         local background_render_object = render_object_create();
         local scroll_y = offy;
         local src_x = render_object_get_src_rect_x(background_render_object);
         while true do
            render_object_set_layer(background_render_object, layer);
            render_object_set_img_id(background_render_object, background_image);
            -- render_object_set_img_id(background_render_object, 0);
            render_object_set_scale(background_render_object, 375, 480);
            render_object_set_position(background_render_object, 0, 0);
            -- render_object_set_layer(background_render_object, 0);

            scroll_y = scroll_y + scrollspeed;

            render_object_set_src_rect(
               background_render_object,
               src_x + offx,
               scroll_y,
               375,
               480
            );

            t_yield();
         end
      end
   )
end

function Generic_Infinite_Stage_ScrollV_BG(image_resource_location, scrollspeed, offx, offy)
   Generic_Infinite_Stage_ScrollV(image_resource_location, scrollspeed, offx, offy, SCRIPTABLE_RENDER_OBJECT_LAYER_BACKGROUND);
end

function Generic_Infinite_Stage_ScrollV_FG(image_resource_location, scrollspeed, offx, offy)
   Generic_Infinite_Stage_ScrollV(image_resource_location, scrollspeed, offx, offy, SCRIPTABLE_RENDER_OBJECT_LAYER_FOREGROUND);
end

-- NOTE(jerry):
-- could cause overflow of scriptable_render_objects?
-- kinda doubt it tbh.
--
-- FADER utility, also an example of how to use
-- the scriptable render object to make some neat
-- effects.
g_black_fader_id = -1;
g_black_fade_out_per_tick = 0.15;
g_black_fade_t = 0.0;
g_fade_direction = 0;
g_black_fade_started = false;

function _fade_black_task()
   while g_black_fade_started do
      local lastalpha = g_black_fade_t;
      if g_fade_direction == 0 then
         -- fade in
         if (lastalpha < 1.0) then
            g_black_fade_t = g_black_fade_t + g_black_fade_out_per_tick;
         else
            -- nothing. Wait until we end.
         end
      else
         -- fade out
         if (lastalpha > 0.0) then
            g_black_fade_t = g_black_fade_t - g_black_fade_out_per_tick;
         else
            g_black_fade_started = false;
         end
      end
      render_object_set_modulation(g_black_fader_id, 0, 0, 0, g_black_fade_t * 0.75);
      t_yield();
   end
end

-- NOTE(jerry):
-- using black fades requires them to be the last item
-- you try to render, but this is a generic one that can be used.
-- Although this technically requires burning render_object_ids per level.
--
-- This one avoids doing it to exhause the scriptable_render_object list.
function start_black_fade(fade_speed)
   if not g_black_fade_started then
      if g_black_fader_id == -1 then
         g_black_fader_id = render_object_create();
         render_object_set_layer(g_black_fader_id, SCRIPTABLE_RENDER_OBJECT_LAYER_BACKGROUND);
         render_object_set_img_id(g_black_fader_id, 0);
         render_object_set_scale(g_black_fader_id, 375, 480);
         render_object_set_position(g_black_fader_id, 0, 0);
         render_object_set_modulation(g_black_fader_id, 0, 0, 0, 0.0);
      end
      g_black_fade_out_per_tick = fade_speed;
      g_black_fade_started = true;
      g_fade_direction = 0;
      async_task_lambda(_fade_black_task);
   end
end

function end_black_fade()
   g_fade_direction = 1;
end
