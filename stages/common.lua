dofile("stages/constants.lua")
-- NOTE
--
-- This Bullet Hell's engine 
-- uses very simple lua. No meta tables please!
--
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

function bullet_position(e)
   return v2(bullet_position_x(e), bullet_position_y(e));
end
function bullet_velocity(e)
   return v2(bullet_velocity_x(e), bullet_velocity_y(e));
end
function bullet_acceleration(e)
   return v2(bullet_acceleration_x(e), bullet_acceleration_y(e));
end

function enemy_position(e)
   return v2(enemy_position_x(e), enemy_position_y(e));
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
   print("test");
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

--
--
-- Will spawn a sprinkle of really dumb pop corn enemies that move linearly
-- 
-- They will look semi random.
--
--
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
