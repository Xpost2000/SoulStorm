dofile("stages/common.lua")
-- Because this is the first level of the bunch
-- it's going to have the worst code since I'm exploring the lua interfacing
-- and also the early lua interface was kinda crap...
--
-- I'm probably not going to add metatables, but I'll hope that future levels are
-- a bit less unwieldy.

local testmusic = load_music("res/snds/8bitinternetoverdose_placeholder.ogg");

--[[
   Stage 1 - 1

   Expected Length : 3.5 minutes?
   Difficulty      : Easy/Medium? Casual

   Author: xpost2000/Jerry Zhu
]]--

function loop_bkg_music()
   while true do
      if not music_playing() then
         play_music(testmusic);
      end
      t_yield();
   end
end

--[[UNIQUE BULLET PATTERNS]]--
function _wave1_bullet_side_shooter_homing_a_little(b)
   bullet_set_velocity(b, 0, 80);
   t_wait(1.54);
   bullet_reset_movement(b);
   t_wait(1.00);

   local t = bullet_time_since_spawn(b);
   while true do
      bullet_start_trail(b, 12);
      bullet_set_trail_modulation(b, 0.8, 0.5, 0.5, 1.0);
      local bx = bullet_position_x(b);
      local by = bullet_position_y(b);
      local px = player_position_x();
      local py = player_position_y();
      local ct = bullet_time_since_spawn(b);
      local dt = ct - t;
      local dir = v2_direction(v2(bx, by), v2(px, py));

      dir[1] = dir[1] * (150 + (dt * 180));
      dir[2] = dir[2] * (150 + (dt * 180));

      bullet_set_velocity(b, dir[1], dir[2]);

      if dt > 0.375 then
         break;
      end

      t_yield();
   end
end

--[[
   MID BOSS?
]]--

--[[
   ENEMY PATTERNS
]]--
-- param0 is for the side of the enemy.
-- 0 = left
-- 1 = right
-- shoot downwards a few times
-- TODO: bind the firing/sound when an enemy shoots.
function _wave1_enemy_shoot_down_and_fly_to_side(e, x_down, y_down, t_mv_speed, side)
   print(e, x_down, y_down, t_mv_speed, side);
   enemy_linear_move_to(e, enemy_position_x(e) + x_down, enemy_position_y(e) + y_down, t_mv_speed);
   local ex = enemy_position_x(e);
   local ey = enemy_position_y(e);
   for bullet=1, 4 do
      local b = bullet_new(BULLET_SOURCE_ENEMY);
      bullet_set_visual(b, PROJECTILE_SPRITE_HOT_PINK_ELECTRIC);
      bullet_set_visual_scale(b, 0.5, 0.5);
      bullet_set_scale(b, 5, 5);
      bullet_set_position(b, ex, ey);
      bullet_set_velocity(b, 0, 100);
      bullet_set_task(b, "_wave1_bullet_side_shooter_homing_a_little");
      -- t_wait(0.30);
      t_wait(0.32);
   end
   t_wait(0.15);

   local sign = 1;
   if side == 0 then
      sign = -1;
   end

   enemy_set_acceleration(e, sign * 150, 125);
   print("SIDE1BYE");
end

-- spinster1 accompanyment
function _wave1_enemy_shoot_down_and_fly_to_side2(e, x_down, y_down, t_mv_speed, side)
   print("SIDE2");
   enemy_linear_move_to(e, enemy_position_x(e) + x_down, enemy_position_y(e) + y_down, t_mv_speed);
   local ex = enemy_position_x(e);
   local ey = enemy_position_y(e);
   for bullet=1, 10 do
      local b = bullet_new(BULLET_SOURCE_ENEMY);
      bullet_set_visual(b, PROJECTILE_SPRITE_HOT_PINK_ELECTRIC);
      bullet_set_visual_scale(b, 0.25, 0.25);
      bullet_set_scale(b, 2, 2);
      bullet_set_position(b, enemy_position_x(e), enemy_position_y(e));
      local d  = v2_direction(v2(ex, ey), v2(play_area_width()/2, play_area_height()*0.75))
      bullet_set_velocity(b, d[1] * 75, d[2] * 75);
      t_wait(0.30);
   end
   print("SIDE2BYELOOP");
   t_wait(0.15);

   local sign = 1;
   if side == 0 then
      sign = -1;
   end

   enemy_set_acceleration(e, sign * 150, 125);
   print("SIDE2BYE");
end

function _wave1_enemy_shoot_down_and_fly_to_side3(e, x_down, y_down, t_mv_speed, side)
   print("SIDE3");
   enemy_linear_move_to(e, enemy_position_x(e) + x_down, enemy_position_y(e) + y_down, t_mv_speed);
   for bullet=1, 15 do
      local b = bullet_new(BULLET_SOURCE_ENEMY);
      bullet_set_visual(b, PROJECTILE_SPRITE_HOT_PINK_ELECTRIC);
      bullet_set_visual_scale(b, 0.25, 0.25);
      bullet_set_scale(b, 2, 2);
      bullet_set_position(b, enemy_position_x(e), enemy_position_y(e));
      local ex = enemy_position_x(e);
      local ey = enemy_position_y(e);
      local d  = v2_direction_from_degree(enemy_time_since_spawn(e) * 16 + 100 * e);
      bullet_set_velocity(b, d[1] * 110, d[2] * 110);
      t_wait(0.25);
   end
   print("SIDE3BYELOOP");
   t_wait(0.15);

   local sign = 1;
   if side == 0 then
      sign = -1;
   end

   enemy_set_acceleration(e, sign * 150, 125);
   print("SIDE3BYE");
end

function _wave1_enemy_sweep_spinster(e, x_down, y_down, t_mv_speed, side)
   enemy_linear_move_to(e, enemy_position_x(e) + x_down, enemy_position_y(e) + y_down, t_mv_speed);
   local fire_delay = 0.10;
   local step       = 10;
   local bspeed     = 100;
   local bacel     =  55;

   for p=0,3 do
      if p == 1 then
         bspeed = bspeed * 1.5;
         bacel = bacel * 1.5;
         fire_delay = fire_delay * 0.5;
      end

      for angle=0, 180, step do
         local d = v2_direction_from_degree(angle);
         local dx = d[1];
         local dy = d[2];
         local b = bullet_new(BULLET_SOURCE_ENEMY);

         bullet_set_visual(b, PROJECTILE_SPRITE_NEGATIVE_ELECTRIC);
         bullet_set_visual_scale(b, 0.5, 0.5);
         bullet_set_scale(b, 5, 5);
         bullet_set_position(b, enemy_position_x(e), enemy_position_y(e));
         bullet_set_velocity(b, dx * bspeed, dy * bspeed);
         bullet_set_acceleration(b, dx * bacel, dy * bacel);

         t_wait(fire_delay);
      end
      for angle=180, 0, -step do
         local d = v2_direction_from_degree(angle);
         local dx = d[1];
         local dy = d[2];
         local b = bullet_new(BULLET_SOURCE_ENEMY);
         bullet_set_visual(b, PROJECTILE_SPRITE_NEGATIVE_ELECTRIC);
         bullet_set_visual_scale(b, 0.5, 0.5);
         bullet_set_scale(b, 5, 5);
         bullet_set_position(b, enemy_position_x(e), enemy_position_y(e));
         bullet_set_velocity(b, dx * bspeed, dy * bspeed);
         bullet_set_acceleration(b, dx * bacel, dy * bacel);

         t_wait(fire_delay);
      end
      t_wait(0.2);
   end
   t_wait(0.18);

   local sign = 1;
   if side == 0 then
      sign = -1;
   end

   enemy_set_acceleration(e, sign * 150, 125);
end

function _wave1_enemy_shotgun_spread(e, x_down, y_down, t_mv_speed, side, shot_times, dx, dy, bullet_variation, cooldown, cooldown2)
   enemy_linear_move_to(e, enemy_position_x(e) + x_down, enemy_position_y(e) + y_down, t_mv_speed);

   bullet_variation = math.floor(bullet_variation);
   for bullet=1, 5 do
      local bullet_set = spawn_bullet_arc_pattern2(
         v2(enemy_position_x(e), enemy_position_y(e)),
         4,
         180,
         v2_normalized(v2(dx, dy)),
         125,
         0,
         BULLET_SOURCE_ENEMY
      );

      if bullet_variation == 0 then
         for bi=1, #bullet_set do
            local b = bullet_set[bi];

            if (bi % 2) == 0 then
               bullet_set_visual(b, PROJECTILE_SPRITE_NEGATIVE_ELECTRIC);
            else
               bullet_set_visual(b, PROJECTILE_SPRITE_RED_ELECTRIC);
            end
            bullet_set_scale(b, 10, 10);
            bullet_set_visual_scale(b, 1.0, 1.0);
         end
      else 
         for bi=1, #bullet_set do
            local b = bullet_set[bi];
            bullet_set_visual(b, PROJECTILE_SPRITE_NEGATIVE_ELECTRIC);
            bullet_set_scale(b, 5, 5);
            bullet_set_visual_scale(b, 0.5, 0.5);
         end
      end

      t_wait(cooldown);
   end
   t_wait(cooldown2);

   local sign = 1;
   if side == 0 then
      sign = -1;
   end

   enemy_set_acceleration(e, sign * 150, 125);
end

--[[
   ENEMY HELPER TEMPLATES
]]--

function _wave1_side_moving_enemy(px, py, xdown, ydown, t_mvspeed, side, variation)
   local e = enemy_new();
   enemy_set_scale(e, 10, 10);
   enemy_set_hp(e, 20);
   enemy_set_position(e, px, py);
   if variation == 0 then
      print("Variation 0 ");
      enemy_set_task(e, "_wave1_enemy_shoot_down_and_fly_to_side", xdown, ydown, t_mvspeed, side);
   elseif variation == 1 then
      print("Variation 1 ");
      enemy_set_task(e, "_wave1_enemy_shoot_down_and_fly_to_side2", xdown, ydown, t_mvspeed, side);
   elseif variation == 2 then
      print("Variation 2 ");
      enemy_set_task(e, "_wave1_enemy_shoot_down_and_fly_to_side3", xdown, ydown, t_mvspeed, side);
   end
   return e;
end

function _wave1_enemy_shotgun_spreader(px, py, xdown, ydown, t_mvspeed, side, firetimes, dx, dy, bullet_variation, cooldown1, cooldown2)
   local e = enemy_new();
   enemy_set_scale(e, 10, 10);
   enemy_set_hp(e, 20);
   enemy_set_position(e, px, py);
   enemy_set_task(e, "_wave1_enemy_shotgun_spread", xdown, ydown, t_mvspeed, side, firetimes, dx, dy, bullet_variation, cooldown1, cooldown2);
   return e;
end

--[[
   NOTE: the play area allows wrapping around the bottom and top of the arena
   Which I want to take advantage of in the gameplay.
]]--
function wave_1_sub1()
   --[[two side enemies]]
   -- shoot bullets that will curve into the player direction
   -- _wave1_side_moving_enemy(100, -100, 0, 200, 1.5, 0, 0);
   _wave1_side_moving_enemy(100, -100, 0, 200, 1.5, 1, 0);
   _wave1_side_moving_enemy(play_area_width() - 100, -100, 0, 200, 1.5, 1, 0);
   t_wait(2.25);
   _wave1_side_moving_enemy(20, -100, 0, 250, 1.5, 0, 0);
   _wave1_side_moving_enemy(play_area_width() - 20, -100, 0, 250, 1.0, 1, 0);
   t_wait(0.5);
   _wave1_side_moving_enemy(50, -100, 0, 250, 1.5, 0, 0);
   _wave1_side_moving_enemy(play_area_width() - 50, -100, 0, 250, 1.0, 1, 0);
   t_wait(1.25);
end

function wave_1_sub2()
   _wave1_enemy_shotgun_spreader(play_area_width()/2 - 10, -10, 0, 70, 0.55, 1, 3,   0, 1, 0, 0.45, 0.25);
    t_wait(4.5);
   _wave1_enemy_shotgun_spreader(play_area_width()/2 - 10, -10, 0, 200, 0.75, 1, 3,  1, 0, 0, 0.75, 0.25);
    t_wait(1.0);
   _wave1_enemy_shotgun_spreader(play_area_width()/2 - 10, -10, 0, 170, 0.75, 0, 3, -1, 0, 0, 0.75, 0.25);
end

function _wave1_enemy_spinster(px, py, xdown, ydown, mvspeed, side)
   local e = enemy_new();
   enemy_set_scale(e, 10, 10);
   enemy_set_position(e, px, py);
   enemy_set_hp(e, 1500);
   enemy_set_task(e, "_wave1_enemy_sweep_spinster", xdown, ydown, mvspeed, side);
end

function wave_1_sub3()
   _wave1_enemy_spinster(play_area_width()/2 -10, -20, 0, 100, 0.35, 1);
   t_wait(0.25);
   for i = 1, 3 do
      _wave1_side_moving_enemy(play_area_width() - 35 * i, -150, 0, 200 + i * 20, 1.5, 1, 1);
   end
   t_wait(0.45);
   for i = 1, 3 do
      _wave1_side_moving_enemy(15 + 35 * i, -150, 0, 200 + i * 20, 1.5, 0, 1);
   end
   t_wait(3.8);
   for i = 1, 4 do
      _wave1_side_moving_enemy(play_area_width() - 35 * i, -150, 0, 200 + i * 20, 1.5, 1, 2);
      _wave1_side_moving_enemy(15 + 35 * i, -150, 0, 200 + i * 20, 1.5, 0, 2);
   end
   t_wait(1.9);
   _wave1_enemy_spinster(play_area_width()/2 -100, -20, -20, 80, 0.4, 0);
   _wave1_enemy_spinster(play_area_width()/2 +100, -20, 20, 80, 0.4, 1);
end

--[I intentionally want this to be the heavier wave and the rest is easier,
--   this wave will repeat encounters, 1x easy the other harder, since this is a simple
--   design system to get around.
--]]
function wave_1()
   wave_1_sub1();
   t_wait(1.67);
   -- [[normal wave shot gun splitters]]
   wave_1_sub2();
   t_wait(1.75);
   -- [[sweeping spinster, to keep you moving left and right.]]
   -- [[NGL, I personally kind of have trouble doing this one but that's cause I'm not good at danmaku]]
   -- [[I can sometimes beat this pattern]]
   wave_1_sub3();
   -- [[the same as the last one but a bit harder]]
end

--[[
   More touhou like design where it's "slower" and allows you to get a chance to score,
   but this will try to have more bullets

   NOTE to self: last pattern should've been the two spinsters
]]-- 
function wave_2_sub1()
   -- Flying rainers
   -- bottom right to top left

   -- a few stray sprinkler hitters

   -- and slower flying rainers
end
function wave_2()
   wave_2_sub1();
end

function wave_3()
end

local midboss_dead = false;

-- Very very simple attack patterns
function mid_boss_minions()
   while midboss_dead == false do
      t_yield();
   end
end

-- slightly less intense spinsters
-- or a few rainers
-- or a few chargers
-- these will occur in patterned waves identified by a table
-- within this coroutine
function mid_boss_wave()
   midboss_dead = true;
end

function stage_task()
   t_wait(1.5);
   -- ideally this should not be a string, but I should allow closures...
   async_task("loop_bkg_music");
   wave_1();

   -- NOTE: the spinsters in wave1 take about 4.? something seconds to
   -- finish their cycles.
   t_wait(3.67);
   wave_2();

   mid_boss();
   async_task("mid_boss_minions");

   wait_no_danger();
   t_complete_stage();
end
