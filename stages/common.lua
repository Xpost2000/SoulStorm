engine_dofile("stages/constants.lua")
-- NOTE
--
-- This Bullet Hell's engine 
-- uses very simple lua. No meta tables please!
--
function normalized_sin(t)
   return (math.sin(t)+1)/2.0;
end
function normalized_cos(t)
   return (math.cos(t)+1)/2.0;
end

-- 0 = left
-- 1 = right
function player_screen_half_horizontal()
   local player_x = player_position_x();
   if player_x <= play_area_width()/2 then
      return 0
   end
   return 1;
end
-- 0 = up
-- 1 = down
function player_screen_half_vertical()
   local player_y = player_position_y();
   if (player_y <= play_area_height()/2) then
      return 0
   end
   return 1
end

function out_of_bounds(position)
   local x = position[1];
   local y = position[2];
   if x < 0 or x >= play_area_width() or
      y < 0 or y >= play_area_heigth() then
      return true;
   end
   return false;
end

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

function wait_no_danger()
      while any_living_danger() do
         t_yield();
      end
end

function bullet_list_set_visuals(bullets, visual)
   for i,b in ipairs(bullets) do
      bullet_set_visual(b, visual);
   end
end
function bullet_list_set_source(bullets, source)
   for i,b in ipairs(bullets) do
      bullet_set_source(b, source);
   end
end
function bullet_list_set_scale(bullets, scale)
   for i,b in ipairs(bullets) do
      bullet_set_scale(b, scale[1], scale[2]);
   end
end

-- Generalish
function player_position()
   return v2(player_position_x(), player_position_y());
end
function dir_to_player(a)
   return v2_direction(a, player_position());
end

function bullet_relative_position(e)
   return v2(bullet_relative_position_x(e), bullet_relative_position_y(e))
end
function bullet_position(e)
   return v2(bullet_position_x(e), bullet_position_y(e));
end
function bullet_final_position(e)
   local relative_position = bullet_relative_position(e);
   local base_position     = bullet_position(e);
   return v2(relative_position[1] + base_position[1], relative_position[2] + base_position[2])
end
function bullet_velocity(e)
   return v2(bullet_velocity_x(e), bullet_velocity_y(e));
end
function bullet_acceleration(e)
   return v2(bullet_acceleration_x(e), bullet_acceleration_y(e));
end

function enemy_relative_position(e)
   return v2(enemy_relative_position_x(e), enemy_relative_position_y(e));
end
function enemy_position(e)
   return v2(enemy_position_x(e), enemy_position_y(e));
end
function enemy_final_position(e)
   local relative_position = enemy_relative_position(e);
   local base_position     = enemy_position(e);
   return v2(relative_position[1] + base_position[1], relative_position[2] + base_position[2])
end
function enemy_velocity(e)
   return v2(enemy_velocity_x(e), enemy_velocity_y(e));
end
function enemy_acceleration(e)
   return v2(enemy_acceleration_x(e), enemy_acceleration_y(e));
end

function dir_to_angle(direction)
   local nd = v2_normalized(direction);
   return math.deg(math.atan(nd[2], nd[1]));
end

--
-- bullet_helpers
--
function bullet_make(src, spawnposition, visual, vscale, scale)
   local b = bullet_new(src);
   bullet_set_visual(b, visual);
   bullet_set_visual_scale(b, vscale[1], vscale[2]);
   bullet_set_scale(b, scale[1], scale[2]);
   bullet_set_position(b, spawnposition[1], spawnposition[2]);
   return b;
end
function bullet_move_linear(b, d, v)
   local d1 = v2_normalized(d);
   bullet_set_velocity(b, d1[1] * v, d1[2] * v);
end

-- This does not really work, or rather the behavior is hard to predict because it's
-- hard to come up with a good function for these.
function bullet_move_asymptopic(e,
                               d,
                               start_velocity,
                               start_acceleration,
                               linear_mvt,
                               flataccel_t,
                               decay_t,
                               retention)
   d = v2_normalized(d);
   bullet_set_velocity(e, start_velocity * d[1], start_velocity * d[2]);
   t_wait(linear_mvt);
   bullet_set_acceleration(e, start_acceleration * d[1], start_acceleration * d[2]);
   t_wait(flataccel_t);
   local st = bullet_time_since_spawn(e);

   while true do
      local dt = bullet_time_since_spawn(e) - st;

      local v = bullet_velocity(e);
      bullet_set_velocity(e, v[1] * retention, v[2] * retention);

      if dt > decay_t then
         bullet_reset_movement(e);
         break;
      end

      t_yield();
   end
end

-- TODO add asympotopic movement?

--
-- enemy_helpers
--

function enemy_linear_move_to(e, x, y, t)
   local cur_x = enemy_position_x(e);
   local cur_y = enemy_position_y(e);

   local dx = (x - cur_x) / t;
   local dy = (y - cur_y) / t;

   enemy_set_velocity(e, dx, dy);
   t_wait(t);
   enemy_reset_movement(e);
end

function enemy_move_linear(e, d, v)
   local d1 = v2_normalized(d);
   enemy_set_velocity(e, d1[1] * v, d1[2] * v);
end

-- I cannot think of a solution for asymptopic movement that is frame
-- Okay, NVM need to reschedule the way tasks are handled.
-- rate independent...
function enemy_move_nonlinear_accel1(e,
                               d,
                               start_velocity,
                               start_acceleration,
                               linear_mvt,
                               flataccel_t)
   d = v2_normalized(d);
   enemy_set_velocity(e, start_velocity * d[1], start_velocity * d[2]);
   t_wait(linear_mvt);
   enemy_set_acceleration(e, start_acceleration * d[1], start_acceleration * d[2]);
   t_wait(flataccel_t);
   enemy_reset_movement(e);
end

-- NOTE: asympotopic movements have to be done manually.
--       no good support for it.

-- As the C API requires a string in it's parameter
-- list, this is a placeholder I can put to avoid making
-- more dedicated functions... This is mostly just to have specific
-- enemies without polluting the "global" namespace.
--
-- Which is a little redundant because I reconstruct a lua state for
-- each level.
function _generic_dispatch(fn, a, b, c, d, e, f, g)
   fn(a,b,c,d,e,f,g)
end

function _generic_entity_task_dispatch(e, fn, a, b, c, d, e1, f, g)
   fn(e, a, b, c, d, e1, f, g);
end

function bullet_task_lambda(e, fn, a, b, c, d, e1, f, g)
   bullet_set_task(e, "_generic_entity_task_dispatch", fn, a, b, c, d, e1, f, g);
end

function enemy_task_lambda(e, fn, a, b, c, d, e1, f, g)
   enemy_set_task(e, "_generic_entity_task_dispatch", fn, a, b, c, d, e1, f, g);
end

function async_task_lambda(fn, a, b, c, d, e, f, g)
   async_task("_generic_dispatch", fn,a,b,c,d,e,f,g);
end

function clamp(x, a, b)
   if (x < a) then
      return a;
   elseif (x > b) then
      return b;
   end
   return x;
end


-- Basic selector function for difficulty settings
-- NOTE: level 1 has no difficulty selection because it's the "introductory" level
-- meant to expose all the mechanics of the game...
function difficulty(a, b, c, d)
   local modifier = get_difficulty_modifier();
   if modifier == 0 then
      return a;
   elseif modifier == 1 then
      return b;
   elseif modifier == 2 then
      return c;
   elseif modifier == 3 then
      return d;
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
            for angle=1,360,angle_spacing do
               local current_arc_direction = v2_direction_from_degree((angle + displacement*displacement_shift) % 360);
               local b = bullet_make(
                  BULLET_SOURCE_ENEMY,
                  enemy_position(e),
                  bullet_visual,
                  v2(0.5, 0.5),
                  v2(5, 5)
               )
               bullet_task_lambda(
                  b,
                  function(b)
                     bullet_set_velocity(b, current_arc_direction[1] * bullet_present_velocity, current_arc_direction[2] * bullet_present_velocity);
                     t_wait(clamp(0.10 * displacement), 0.1, 0.5);
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

function Enemy_AddExplodeOnDeathBehavior(e, explosion_radius, warning_timer, explosion_timer)
   async_task_lambda(
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
      end,
      e
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
      sign_modh)
   local enemies = {};
   for i=1, amount do
      local e = enemy_new();
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
                                  bullet_visual
                                 )
   local e = enemy_new();
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

-- This is a very bullet hell style attack
-- obviously it's pretty hard to dodge a lot of this, so I have to be pretty careful.
-- I've concluded it *is* possible to dodge but it's really hard, and I might need to reduce some visual noise.

-- RAIN STORM ATTACK: try to dodge
function MainBoss1_RainCloud_Attack1(phase_cycle, duration)
   local stop_task = 0;

   local spread = 40;
   local x_adv = play_area_width() / (spread - 5);

   async_task_lambda(
      function()
         disable_grazing();
         disable_bullet_to_points();

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
                  for i=1, 40 do
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
                  t_wait(0.354);
               end
            end
         )

         t_wait(duration)
         stop_task = 1;
         enable_grazing();
         enable_bullet_to_points();
      end
   )
end

function MainBoss1_RainCloud_Attack2(phase_cycle, duration)
   local stop_task = 0;

   local spread = 35;
   local x_adv = play_area_width() / (spread);

   async_task_lambda(
      function()
         disable_grazing();
         disable_bullet_to_points();
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
                  for i=1, 35 do
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
                  t_wait(0.200);
               end
            end
         )
         t_wait(duration)
         enable_grazing();
         enable_bullet_to_points();
         stop_task = 1;
      end
   )
end
