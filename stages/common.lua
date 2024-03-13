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

-- Dramatic opening
function Stage1_Batflood()
   -- Bat flood
   -- free popcorn points
   local wave_count = 15;
   local per_wave   = 15;
   for i=1,wave_count do
      for j=1,per_wave do
         local e = enemy_new();
         enemy_set_hp(e, 5);
         enemy_set_position(e, j * (play_area_width() / (per_wave)) - 18, -30);
         enemy_task_lambda(
            e,
            function (e)
               enemy_set_velocity(e, 0, 125);
               t_wait(1.0);
               if j > per_wave/2 then
                  enemy_set_acceleration(e, 40, 0);
               else
                  enemy_set_acceleration(e, -40, 0);
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
function Generic_Infinite_Stage_ScrollV(image_resource_location, scrollspeed)
   async_task_lambda(
      function ()
         local background_image         = load_image(image_resource_location);
         local background_render_object = render_object_create();
         local scroll_y = 0;
         while true do
            render_object_set_img_id(background_render_object, background_image);
            -- render_object_set_img_id(background_render_object, 0);
            render_object_set_scale(background_render_object, 375, 480);
            render_object_set_position(background_render_object, 0, 0);
            render_object_set_layer(background_render_object, 0);

            local src_x = render_object_get_src_rect_x(background_render_object);

            scroll_y = scroll_y + scrollspeed;
            render_object_set_src_rect(
               background_render_object,
               src_x,
               scroll_y,
               375,
               480
            );

            t_wait(2.0);
            t_yield();
         end
      end
   )
end
