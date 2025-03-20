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
      y < 0 or y >= play_area_height() then
      return true;
   end
   return false;
end


function normalized_sin(t)
   return (math.sin(t)+1)/2.0;
end
function normalized_cos(t)
   return (math.cos(t)+1)/2.0;
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

function v2_equal(a, b)
   local dx = math.abs(b[1] - a[1]);
   local dy = math.abs(b[2] - a[2]);

   if dx < 0.005 and dy < 0.005 then
      return true;
   end
   return false;
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

function v2_screen_middle()
   return v2(play_area_width()/2, play_area_height()/2);
end

--
-- Asynchronous stage task queue
--
--
-- "Scriptland" helper API to make level scripting a little easier based on absolute
--  timings instead of relative timings.
--
--
asynchronous_stage_task_queue = {};
function absolute_stage_scheduler_run()
   async_task_lambda(
      function()
         while true do
            local current_time      = current_stage_time();
            local to_remove_indices = {};

            for k, v in pairs(asynchronous_stage_task_queue) do
               local trigger_time = v[1];
               if trigger_time >= current_time then
                  table.insert(to_remove_indices, k);
                  local fn = v[2];
                  local a  = v[3];
                  local b  = v[4];
                  local c  = v[5];
                  local d  = v[6];
                  local e  = v[7];
                  local f  = v[8];
                  local g  = v[9];
                  fn(a, b, c, d, e, f, g);
               end
            end

            for k, v in pairs(to_remove_indices) do
               table.remove(asynchronous_stage_task_queue, v);
            end

            t_yield();
         end
      end
   );
end

function stage_async_task_at(absolute_time_in_seconds, fn, a, b, c, d, e, f, g)
   entry = {
      -- timer
      absolute_time_in_seconds,
      -- callback
      fn,
      -- params
      a, b, c, d, e, f, g
   };
   table.insert(asynchronous_stage_task_queue, entry);
end
