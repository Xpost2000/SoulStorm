dofile("stages/common.lua")
-- TODO:
-- Prettification pass.

-- Because this is the first level of the bunch
-- it's going to have the worst code since I'm exploring the lua interfacing
-- and also the early lua interface was kinda crap...
-- also refactoring/rewriting is not a goal
-- it takes a long time to test the pattern, and rewriting the old patterns could mean I break
-- them soo............. sorry for the next reader.
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
      play_sound(random_attack_sound());
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
      play_sound(random_attack_sound());
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
      play_sound(random_attack_sound());
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
   local bspeed     = 90;
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

         play_sound(random_attack_sound());
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

         play_sound(random_attack_sound());
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

      play_sound(random_attack_sound());
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
   enemy_set_hp(e, 27);
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
   enemy_set_hp(e, 27);
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
   enemy_set_hp(e, 2500);
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
function _wave2_enemy_rainer_sprinkler1(e, i, max_i, base_wait, wait_phase_before_turn, fire_proc)
   enemy_set_velocity(e, 0, -235);
   t_wait(base_wait + i * wait_phase_before_turn);
   local v_mod = (max_i - i) * 10;
   for k = 0,90,10 do
      local d = v2_direction_from_degree(90 + k);
      d[2] = d[2] * -1;
      enemy_set_velocity(e, (120+(v_mod)) * d[1], (120+(v_mod)) * d[2]);
      t_wait(0.10);
   end
  -- start shooting 
  fire_proc(e, (max_i - i));
end

function wave_2_sub1()
   -- Flying rainers
   -- bottom right to top left

   -- These basic rainers will sprinkle bullets downwards

   -- this one will shoot faster big shots
   -- arc2
   for i=1,5 do
      local e = enemy_new();
      enemy_set_scale(e, 10, 10);
      enemy_set_hp(e, 20);
      enemy_set_position(e, play_area_width() - 30, play_area_height() + 35 + (35 * i));
      enemy_set_task(
         e, "_wave2_enemy_rainer_sprinkler1", i, 5, 1.4, 0.23,

         function (e, idx)
            for i=1, 10 do
               local ep = v2(enemy_position_x(e), enemy_position_y(e));
               local bullets = spawn_bullet_arc_pattern2(
                  ep,
                  3,
                  90,
                  v2(0, 1),
                  75,
                  0,
                  BULLET_SOURCE_ENEMY
               );
               play_sound(random_attack_sound());

               bullet_list_set_visuals(
                  bullets,
                  PROJECTILE_SPRITE_HOT_PINK_ELECTRIC
               );

               for i,b in ipairs(bullets) do
                  bullet_set_visual_scale(b, 0.25, 0.25);
                  bullet_set_scale(b, 2.5, 2.5);
               end

               t_wait(0.10);
            end
         end
      );
   end
   t_wait(1.75)

   -- this one shoots slower waves
   -- like a mini version of the sprinkler.
   for i=1,5 do
      local e = enemy_new();
      enemy_set_scale(e, 10, 10);
      enemy_set_hp(e, 25);
      enemy_set_position(e, play_area_width() - 30, play_area_height() + 35 + (35 * i));
      enemy_set_task(
         e, "_wave2_enemy_rainer_sprinkler1", i, 5, 1.1, 0.27,
         function (e, idx)
            for i=1, 3 do
               local ep = v2(enemy_position_x(e), enemy_position_y(e));
               local bullets = spawn_bullet_arc_pattern2(
                  ep,
                  5,
                  90,
                  v2(0, 1),
                  170,
                  0,
                  BULLET_SOURCE_ENEMY
               );
               play_sound(random_attack_sound());

               bullet_list_set_visuals(
                  bullets,
                  PROJECTILE_SPRITE_NEGATIVE_ELECTRIC
               );

               for i,b in ipairs(bullets) do
                  bullet_set_visual_scale(b, 0.50, 0.50);
                  bullet_set_scale(b, 5, 5);
               end

               t_wait(0.4);
            end
         end
      );
   end


   -- a few stray sprinkler hitters

   -- and slower flying rainers
end

function _wave2_enemy_dipshooter(e, dip_divisions, dip_height, fire_on_dip, dip_t, dip_delay, fire_f1, fire_f2)
   local dip_x_adv = play_area_width() / dip_divisions;

   for dip=1,dip_divisions do
      t_wait(dip_delay);

      local ex = enemy_position_x(e);
      local ey = enemy_position_y(e);
      local modifier = 1;

      if (dip % 2) == 0 then
         modifier = -1;
         fire_f1(e)
      else
         fire_f2(e)
      end

      enemy_linear_move_to(e, ex + dip_x_adv, ey + dip_height * modifier, dip_t)
      t_yield();
   end
   -- NOTE: it turns out one of the dipshooters didn't exit the stage properly. Whoops!
   enemy_set_velocity(e, 100);
end

function wave_2_sub2()
   -- some dip shooters come in
   do 
      local e = enemy_new();
      enemy_set_scale(e, 10, 10);
      enemy_set_hp(e, 20);
      enemy_set_position(e, 0, 100);
      enemy_set_task(
         e, "_wave2_enemy_dipshooter", 7, 50, 1, 0.402, 0.35,
         function (e)
            for i=1, 4 do
               local ep = v2(enemy_position_x(e), enemy_position_y(e));
               local bullets = spawn_bullet_arc_pattern2(
                  ep,
                  5,
                  90,
                  v2(0, 1),
                  120,
                  0,
                  BULLET_SOURCE_ENEMY
               );
               play_sound(random_attack_sound());

               bullet_list_set_visuals(
                  bullets,
                  PROJECTILE_SPRITE_RED_ELECTRIC
               );

               for i,b in ipairs(bullets) do
                  bullet_set_visual_scale(b, 0.50, 0.50);
                  bullet_set_scale(b, 5, 5);
               end

               t_wait(0.25);
            end
         end,
         function (e)
         end
      );
   end

   for i = 1, 5 do
      _wave1_side_moving_enemy(-100, -175, 100 + i * 50, 200, 1.6, 0, 0);
      t_wait(0.17);
   end

   for i1=1, 4 do 
      local e = enemy_new();
      enemy_set_scale(e, 10, 10);
      enemy_set_hp(e, 20);
      enemy_set_position(e, 0 + -i1 * 5, 30);
      enemy_set_task(
         e, "_wave2_enemy_dipshooter", 6, 80, 1, 0.250, 0.35,
         function (e)
         end,
         function (e)
            for i=1, 3 do
               local ep = v2(enemy_position_x(e), enemy_position_y(e));
               local d1 = v2_direction_from_degree(145 + 90);
               d1[1]  = d1[1] * -1;
               local d2 = v2_direction_from_degree(15 + 90);
               d1[2]  = d1[2] * -1;
               do 
                  local bullets = spawn_bullet_arc_pattern2(
                     ep,
                     3,
                     15,
                     d1,
                     150,
                     0,
                     BULLET_SOURCE_ENEMY
                  );
                  play_sound(random_attack_sound());

                  bullet_list_set_visuals(
                     bullets,
                     PROJECTILE_SPRITE_NEGATIVE_ELECTRIC
                  );

                  for i,b in ipairs(bullets) do
                     bullet_set_visual_scale(b, 0.25, 0.25);
                     bullet_set_scale(b, 2.5, 2.5);
                     bullet_start_trail(b, 12);
                  end
               end
               do 
                  local bullets = spawn_bullet_arc_pattern2(
                     ep,
                     3,
                     15,
                     d2,
                     150,
                     0,
                     BULLET_SOURCE_ENEMY
                  );
                  play_sound(random_attack_sound());

                  bullet_list_set_visuals(
                     bullets,
                     PROJECTILE_SPRITE_NEGATIVE_ELECTRIC
                  );

                  for i,b in ipairs(bullets) do
                     bullet_set_visual_scale(b, 0.25, 0.25);
                     bullet_set_scale(b, 2.5, 2.5);
                     bullet_start_trail(b, 15);
                  end
               end

               t_wait(0.25);
            end
         end
      );
      t_wait(1.5)
   end
   -- fireworks1 and fireworks2 (outer star shot)
   -- fireworks3 and fireworks4
end

-- Simple pattern. Fire the pattern but slightly delayed between
-- shots.
-- Bullets will accelerate a little. Stop, then move out.
-- this is just for visuals, but functionally it makes no difference
function _wave2_bullet_firework1(b, arc_direction, start_speed, setup_t, acceleration_speed)
   bullet_set_velocity(b, arc_direction[1] * start_speed, arc_direction[2] * start_speed);
   t_wait(setup_t);
   bullet_reset_movement(b);
   bullet_set_acceleration(b, arc_direction[1] * acceleration_speed, arc_direction[2] * acceleration_speed);
end

-- Almost the same as the last one. But will spin for a bit before firing with the spin.
function _wave2_bullet_firework2(b, arc_direction, start_speed, setup_t, acceleration_speed)
   local sp = v2(bullet_position_x(b), bullet_position_y(b));

   bullet_set_velocity(b, arc_direction[1] * start_speed, arc_direction[2] * start_speed);
   t_wait(setup_t);
   bullet_reset_movement(b);

   local ep = v2(bullet_position_x(b), bullet_position_y(b));
   local r = v2_distance(sp, ep);
   do
      local st = bullet_time_since_spawn(b);
      local starting_angle = math.atan(arc_direction[2], arc_direction[1]);

      while bullet_time_since_spawn(b) - st < 1.45 do
         dt = bullet_time_since_spawn(b) - st;
         bullet_set_velocity(
            b,
            -math.sin(starting_angle + dt*(dt+1)) * r,
            math.cos(starting_angle + dt*(dt+1)) * r
         );
         t_yield();
      end
   end
   t_wait(0.15);
   -- I'mma be honest. I don't know why this looks so cool.
   bullet_set_acceleration(b, arc_direction[1] * acceleration_speed, arc_direction[2] * acceleration_speed);
end

function _wave2_enemy_firework(e, accel, wait_t, wait_t2, shot_spreads, delay_between_spreads, variation)
   enemy_set_acceleration(e, accel[1], accel[2]);
   t_wait(wait_t);
   enemy_reset_movement(e);

   local ex = enemy_position_x(e);
   local ey = enemy_position_y(e);
   for j=0,(shot_spreads-1) do
      local displacement = j * 16.5;

      -- NOTE: if an enemy dies before their task "finishes"
      -- we might get "ghost bullets that freeze"
      for angle=1,360,40 do
         local aangle = (angle + displacement) % 360;
         local current_arc_direction = v2_direction_from_degree((angle + displacement) % 360);

         local nb = bullet_new(BULLET_SOURCE_ENEMY);
         bullet_set_task(nb, variation, current_arc_direction, 90, 0.25 + (j+1)*0.15, 800);
         bullet_set_position(nb, ex, ey);
         bullet_set_scale(nb, 5, 5);
         if not (variation == "_wave2_bullet_firework1") then
            bullet_set_visual(nb, PROJECTILE_SPRITE_NEGATIVE_ELECTRIC);
         else
            bullet_set_visual(nb, PROJECTILE_SPRITE_RED_ELECTRIC);
         end
         bullet_set_visual_scale(nb, 0.5, 0.5);
         bullet_set_lifetime(nb, 10);
         play_sound(random_attack_sound());
      end

      t_wait(delay_between_spreads);
   end
   t_wait(wait_t2);
   enemy_set_acceleration(e, -100, -100);
end

function wave_2_sub3()
   do
      local dist_r = 100;
      do
         local e = enemy_new();
         enemy_set_scale(e, 10, 10);
         enemy_set_hp(e, 45);
         enemy_set_position(e, play_area_width()/2 - dist_r, -50);
         enemy_set_task(
            e,
            "_wave2_enemy_firework", v2(0, 100), 1.5, 4, 12, 0.45, "_wave2_bullet_firework1"
         );
      end
      do
         local e = enemy_new();
         enemy_set_scale(e, 10, 10);
         enemy_set_hp(e, 45);
         enemy_set_position(e, play_area_width()/2 + dist_r, -50);
         enemy_set_task(
            e,
            "_wave2_enemy_firework", v2(0, 100), 1.5, 4, 12, 0.45, "_wave2_bullet_firework1"
         );
      end
   end
   t_wait(1.25)
   do
      local e = enemy_new();
      enemy_set_scale(e, 10, 10);
      enemy_set_hp(e, 75);
      enemy_set_position(e, play_area_width()/2, -50);
      enemy_set_task(
         e,
         "_wave2_enemy_firework", v2(0, 100), 1.2, 6, 25, 0.45, "_wave2_bullet_firework2"
      );
   end
   t_wait(5.35)
   do
      local dist_r = 40;
      do
         local e = enemy_new();
         enemy_set_scale(e, 10, 10);
         enemy_set_hp(e, 45);
         enemy_set_position(e, play_area_width()/2 - dist_r, -20);
         enemy_set_task(
            e,
            "_wave2_enemy_firework", v2(0, 50), 1.5, 4, 6, 0.25, "_wave2_bullet_firework1"
         );
      end
      do
         local e = enemy_new();
         enemy_set_scale(e, 10, 10);
         enemy_set_hp(e, 45);
         enemy_set_position(e, play_area_width()/2 + dist_r, -20);
         enemy_set_task(
            e,
            "_wave2_enemy_firework", v2(0, 50), 1.5, 4, 20, 0.15, "_wave2_bullet_firework1"
         );
      end
   end
   t_wait(8.5)

   -- free bird flock for points + (padding time)
   -- okay some of them sprinkle a little something
   for enemy=1, 35 do
      local e = enemy_new();
      enemy_set_scale(e, 10, 10);
      enemy_set_hp(e, 25);
      local sign = enemy%3;
      if (sign < 1) then
         sign = -1;
      else
         sign = 1;
      end
      enemy_set_position(e, play_area_width() + 15*enemy, play_area_height()+30 + enemy * sign * 5);
      -- enemy_set_acceleration(e, -170, -280);
      enemy_set_velocity(e, -70, -100);
   end

   for i=1, 4 do
      local d = v2_direction_from_degree(47);
      _wave1_enemy_shotgun_spreader(-30, -20, 50, 50 + i*55, 2.0, 1, 3, d[1], d[2], 1, 0.15, 4.5);
      t_wait(0.15)
   end

   t_wait_for_no_danger()
end


-- X Cross pattern (very easy)
-- modified spinster pattern
function _wave2_sub4_xcross_fire(e, xsgn, ascension_speed)
   local direction = v2_direction_from_degree(45);
   print(e, direction, xsgn, ascension_speed)
   print(e, "move");
   enemy_set_velocity(e, xsgn * 20, 0);
   t_wait(4.0);
   print(e, "wait");
   enemy_reset_movement(e);
   t_wait(1.0);
   print(e, "climbcrossdeath");
   enemy_set_velocity(e, 0, -1 * ascension_speed);

   while enemy_valid(e) do
      local b = bullet_new(BULLET_SOURCE_ENEMY);
      local ex = enemy_position_x(e);
      local ey = enemy_position_y(e);

      bullet_set_position(b, ex, ey);
      bullet_set_visual(b, PROJECTILE_SPRITE_RED_ELECTRIC);
      bullet_set_scale(b, 2.5, 2.5);
      bullet_set_visual_scale(b, 0.25, 0.25);
      bullet_set_velocity(b, xsgn * 100 * direction[1], direction[2] * 100);

      print(e, "boom boom!");
      play_sound(random_attack_sound());
      t_wait(0.15);
   end
end

-- NOTE: this requires exploiting the fact the play area wraps in order to beat
-- which is a unique thing for some of the levels in this game.
-- If you didn't discover it already, this pattern should force you to do so...
function wave_2_sub4()
   for i=1, 5 do
      local r = 25;
      local y = 45 * i;
      do
        local e = enemy_new();
        enemy_set_scale(e, 10, 10);
        enemy_set_hp(e, 35);
        enemy_set_position(e, -r, y);
        enemy_set_task(e, "_wave2_sub4_xcross_fire", 1, 50);
      end
      do
        local e = enemy_new();
        enemy_set_scale(e, 10, 10);
        enemy_set_hp(e, 35);
        enemy_set_position(e, play_area_width() + r, y);
        enemy_set_task(e, "_wave2_sub4_xcross_fire", -1, 50);
      end
   end

   t_wait_for_no_danger();
   t_wait(15);

   for i=1, 15, 5 do
      local r = 25;
      local y = 35 * i + 20;
      do
        local e = enemy_new();
        enemy_set_scale(e, 10, 10);
        enemy_set_hp(e, 35);
        enemy_set_position(e, -r, y);
        enemy_set_task(e, "_wave2_sub4_xcross_fire", 1, 100);
      end
      do
        local e = enemy_new();
        enemy_set_scale(e, 10, 10);
        enemy_set_hp(e, 35);
        enemy_set_position(e, play_area_width() + r, y);
        enemy_set_task(e, "_wave2_sub4_xcross_fire", -1, 100);
      end
   end
end

function _wave2_sub5_goon_initial(e, goon_number)
   -- these goons will spawn an arc pointed towards the center line
   -- that will delay before initiating.
   -- side goons shoot a bit faster.
   local eposition = v2(enemy_position_x(e), enemy_position_y(e));

   -- I'm aware this can be copied and pasted
   -- but cleaning this up is more trouble than it's worth. LOL
   if goon_number == 0 or goon_number == 2 then
      local new_bullets = {};
      local arcs1       = {};

      local new_bullets2 = {};
      local arcs2       = {};

      local new_bullets3 = {};
      local arcs3       = {};

      local bcount_per_wave = 20;

      local sgn = 1;
      if (goon_number == 2) then
         sgn = -1;
      end

      local arc_sub_length = 180 / bcount_per_wave;
      for i=1, bcount_per_wave do
         local angle = dir_to_angle(v2(1*sgn, 1)) + arc_sub_length * ((i-1) - (bcount_per_wave/2));
         local current_arc_direction = v2_direction_from_degree(angle);
         local position = v2_add(eposition, v2(current_arc_direction[1] * 45, current_arc_direction[2] * 45))

         local nb = bullet_new(BULLET_SOURCE_ENEMY);
         new_bullets[i] = nb;
         arcs1[i] = current_arc_direction;
         bullet_set_visual(nb, PROJECTILE_SPRITE_HOT_PINK_ELECTRIC);
         bullet_set_position(nb, position[1], position[2]);
         bullet_set_scale(nb, 2.5, 2.5);
         bullet_set_visual_scale(nb, 0.25, 0.25);
         bullet_set_lifetime(nb, 25);
         play_sound(random_attack_sound());
         t_wait(0.025);
      end

      for i=1, bcount_per_wave do
         local angle = dir_to_angle(v2(1*sgn, 1))-10 + arc_sub_length * ((i-1) - (bcount_per_wave/2));
         local current_arc_direction = v2_direction_from_degree(angle);
         local position = v2_add(eposition, v2(current_arc_direction[1] * 70, current_arc_direction[2] * 70))

         local nb = bullet_new(BULLET_SOURCE_ENEMY);
         new_bullets2[i] = nb;
         arcs2[i] = current_arc_direction;
         bullet_set_visual(nb, PROJECTILE_SPRITE_HOT_PINK_ELECTRIC);
         bullet_set_position(nb, position[1], position[2]);
         bullet_set_scale(nb, 2.5, 2.5);
         bullet_set_visual_scale(nb, 0.25, 0.25);
         bullet_set_lifetime(nb, 25);
         play_sound(random_attack_sound());
         t_wait(0.025);
      end

      for i=1, bcount_per_wave do
         local angle = dir_to_angle(v2(1*sgn, 1))-10 + arc_sub_length * ((i-1) - (bcount_per_wave/2));
         local current_arc_direction = v2_direction_from_degree(angle);
         local position = v2_add(eposition, v2(current_arc_direction[1] * 90, current_arc_direction[2] * 90))

         local nb = bullet_new(BULLET_SOURCE_ENEMY);
         new_bullets3[i] = nb;
         arcs3[i] = current_arc_direction;
         bullet_set_visual(nb, PROJECTILE_SPRITE_HOT_PINK_ELECTRIC);
         bullet_set_position(nb, position[1], position[2]);
         bullet_set_scale(nb, 2.5, 2.5);
         bullet_set_visual_scale(nb, 0.25, 0.25);
         bullet_set_lifetime(nb, 25);
         play_sound(random_attack_sound());
         t_wait(0.025);
      end

      t_wait(1);
      for i=1,bcount_per_wave do
         bullet_set_velocity(new_bullets[i],
                             arcs1[i][1] * 45,
                             arcs1[i][2] * 45 
         );
      end
      t_wait(0.5);
      for i=1,bcount_per_wave do
         bullet_set_velocity(new_bullets2[i],
                             arcs2[i][1] * 65,
                             arcs2[i][2] * 65 
         );
      end
      t_wait(0.5);
      for i=1,bcount_per_wave do
         bullet_set_velocity(new_bullets3[i],
                             arcs3[i][1] * 35,
                             arcs3[i][2] * 35 
         );
      end
      t_wait(0.470)
      enemy_set_velocity(e, 0, 100);
   elseif goon_number == 1 then
      -- This one will vomit out a mass of bullets
      -- that are slow, and will last a fair while.
      for i=1, 45 do
         local ep = v2(enemy_position_x(e), enemy_position_y(e));
         local bullets = spawn_bullet_arc_pattern2(
            ep,
            35,
            100-i,
            v2(0, 1),
            45,
            0,
            BULLET_SOURCE_ENEMY
         );
         play_sound(random_attack_sound());

         bullet_list_set_visuals(
            bullets,
            PROJECTILE_SPRITE_NEGATIVE_ELECTRIC
         );

         for i,b in ipairs(bullets) do
            bullet_set_visual_scale(b, 0.15, 0.15);
            bullet_set_scale(b, 1.5, 1.5);
         end

         t_wait(0.177);
      end
      t_wait(0.250)
      enemy_set_velocity(e, 100, 100);
   end
end

function wave_2_sub5()
   t_wait(1.0)
   do
      -- TODO: Fade these guys in.
      local goon = {};
      goon[1] = enemy_new();
      enemy_set_scale(goon[1], 10, 10);
      enemy_set_hp(goon[1], 125);
      enemy_set_position(goon[1], 50, 80);
      t_wait(1.0);
      goon[2] = enemy_new();
      enemy_set_scale(goon[2], 10, 10);
      enemy_set_hp(goon[2], 5000); -- He's making the main puzzle pattern.
      enemy_set_position(goon[2], play_area_width()/2, 30);
      t_wait(1.0);
      goon[3] = enemy_new();
      enemy_set_scale(goon[3], 10, 10);
      enemy_set_hp(goon[3], 125);
      enemy_set_position(goon[3], play_area_width() - 50, 80);
      t_wait(1.0);

      enemy_set_task(goon[1], "_wave2_sub5_goon_initial", 0);
      enemy_set_task(goon[3], "_wave2_sub5_goon_initial", 2);
      t_wait(1.0);
      enemy_set_task(goon[2], "_wave2_sub5_goon_initial", 1);
      t_wait(1.0);
   end
   t_wait(3.25);
   do
      local slice_y = play_area_height()/8;
      for i=1, 8 do
         local new_bullet = bullet_new(BULLET_SOURCE_ENEMY);
         bullet_set_position(new_bullet, -10, (i-1) * slice_y);
         bullet_set_velocity(new_bullet, 50, 0);
         bullet_set_visual(new_bullet, PROJECTILE_SPRITE_BLUE);
         bullet_set_visual_scale(new_bullet, 0.5, 0.5);
         bullet_set_scale(new_bullet, 5, 5);
      end

      for i=1, 8 do
         local new_bullet = bullet_new(BULLET_SOURCE_ENEMY);
         bullet_set_position(new_bullet, play_area_width()+10, (i-3) * slice_y);
         bullet_set_velocity(new_bullet, -50, 0);
         bullet_set_visual(new_bullet, PROJECTILE_SPRITE_BLUE);
         bullet_set_visual_scale(new_bullet, 0.5, 0.5);
         bullet_set_scale(new_bullet, 5, 5);
      end
   end
   t_wait(3.5);
   do
      local slice_y = play_area_height()/8;
      for i=1, 8 do
         local new_bullet = bullet_new(BULLET_SOURCE_ENEMY);
         bullet_set_position(new_bullet, -10, (i-3) * slice_y);
         bullet_set_velocity(new_bullet, 50, 0);
         bullet_set_visual(new_bullet, PROJECTILE_SPRITE_BLUE);
         bullet_set_visual_scale(new_bullet, 0.5, 0.5);
         bullet_set_scale(new_bullet, 5, 5);
      end

      for i=1, 8 do
         local new_bullet = bullet_new(BULLET_SOURCE_ENEMY);
         bullet_set_position(new_bullet, play_area_width()+10, (i-1) * slice_y);
         bullet_set_velocity(new_bullet, -50, 0);
         bullet_set_visual(new_bullet, PROJECTILE_SPRITE_BLUE);
         bullet_set_visual_scale(new_bullet, 0.5, 0.5);
         bullet_set_scale(new_bullet, 5, 5);
      end
   end
   t_wait(3.5);
   do
      local slice_y = play_area_height()/8;
      for i=1, 8 do
         local new_bullet = bullet_new(BULLET_SOURCE_ENEMY);
         bullet_set_position(new_bullet, -10, (i-1) * slice_y);
         bullet_set_velocity(new_bullet, 50, 0);
         bullet_set_visual(new_bullet, PROJECTILE_SPRITE_BLUE);
         bullet_set_visual_scale(new_bullet, 0.5, 0.5);
         bullet_set_scale(new_bullet, 5, 5);
      end

      for i=1, 8 do
         local new_bullet = bullet_new(BULLET_SOURCE_ENEMY);
         bullet_set_position(new_bullet, play_area_width()+10, (i-3) * slice_y);
         bullet_set_velocity(new_bullet, -50, 0);
         bullet_set_visual(new_bullet, PROJECTILE_SPRITE_BLUE);
         bullet_set_visual_scale(new_bullet, 0.5, 0.5);
         bullet_set_scale(new_bullet, 5, 5);
      end
   end
end

function wave_2_sub6_generic_attacker1(e, dir)
   while enemy_valid(e) do
      local bullets = spawn_bullet_line(enemy_position(e), 3, 10, v2(10,10), v2(0, 1), 100, BULLET_SOURCE_ENEMY);
      t_wait(0.55);
      play_sound(random_attack_sound());
   end
end

function wave_2_sub_6()
   
   -- some basic dummy enemies that exist to be shot

   do

      for i=1, 12 do
         local e = enemy_new();
         enemy_set_scale(e, 10, 10);
         enemy_set_hp(e, 10);
         enemy_set_position(e, play_area_width()-50, -20);
         enemy_set_velocity(e, 0, 95);
         enemy_set_acceleration(e, -100, 5);
         t_wait(0.20);
      end
      t_wait(1.2);

      -- spawn some actual aggressors.
      for i=1, 7 do
         local e = enemy_new();
         enemy_set_scale(e, 10, 10);
         enemy_set_hp(e, 15);
         enemy_set_position(e, play_area_width()/2, -10);
         enemy_set_velocity(e, 0, 80);

         if (i <= 3) then
            enemy_set_acceleration(e, -120, 25);
            enemy_set_task(e, "wave_2_sub6_generic_attacker1", 0);
         else
            enemy_set_acceleration(e, 120, 25);
            enemy_set_task(e, "wave_2_sub6_generic_attacker1", 1);
         end
      end

      for i=1, 12 do
         local e = enemy_new();
         enemy_set_scale(e, 10, 10);
         enemy_set_hp(e, 20);
         enemy_set_position(e, 90, -30);
         enemy_set_velocity(e, 0, 75);
         enemy_set_acceleration(e, 100, 10);
         t_wait(0.20);
      end

      t_wait(1);

      for i=1,5 do
         local e = enemy_new();
         enemy_set_scale(e, 10, 10);
         enemy_set_hp(e, 20);
         enemy_set_position(e, play_area_width() - 30, play_area_height() + 35 + (30 * i));
         enemy_set_task(
            e, "_wave2_enemy_rainer_sprinkler1", i, 5, 1.4, 0.23,

            function (e, idx)
               for i=1, 10 do
                  local ep = v2(enemy_position_x(e), enemy_position_y(e));
                  local bullets = spawn_bullet_arc_pattern2(
                     ep,
                     3,
                     90,
                     v2(0, 1),
                     75,
                     0,
                     BULLET_SOURCE_ENEMY
                  );
                  play_sound(random_attack_sound());

                  bullet_list_set_visuals(
                     bullets,
                     PROJECTILE_SPRITE_HOT_PINK_ELECTRIC
                  );

                  for i,b in ipairs(bullets) do
                     bullet_set_visual_scale(b, 0.25, 0.25);
                     bullet_set_scale(b, 2.5, 2.5);
                  end

                  t_wait(0.10);
               end
            end
         );
      end

      t_wait(1.5);
      do
         local dist_r = 100;
         do
            local e = enemy_new();
            enemy_set_scale(e, 10, 10);
            enemy_set_hp(e, 45);
            enemy_set_position(e, play_area_width()/2 - dist_r, -50);
            enemy_set_task(
               e,
               "_wave2_enemy_firework", v2(0, 100), 1.5, 2, 6, 0.65, "_wave2_bullet_firework1"
            );
         end
      do
         local e = enemy_new();
         enemy_set_scale(e, 10, 10);
         enemy_set_hp(e, 45);
         enemy_set_position(e, play_area_width()/2 + dist_r, -50);
         enemy_set_task(
            e,
            "_wave2_enemy_firework", v2(0, 100), 1.5, 2, 6, 0.65, "_wave2_bullet_firework1"
         );
      end
   end

   end
end

function wave_2()
   wave_2_sub1();
   t_wait(6.4);
   wave_2_sub2();
   t_wait(3.5)
   wave_2_sub3();
   -- safe buffer for five seconds.
   t_wait(5);
   wave_2_sub4();
   t_wait_for_no_danger();
   t_wait(7.5);

   -- spawn 3 enemies that will start blanketing
   wave_2_sub5();
   t_wait(11.0);

   -- here are some standard shmup waves.
   wave_2_sub2();
   t_wait(10.0);
   wave_2_sub_6();
end

function blanket_wave1()
end
function blanket_wave2()
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

-- NOTE: I have not introduced explosions or lasers
-- in this level yet!
--
-- I will introduce them in 1-3 as a new gameplay "mechanic"
--
function stage_task()
   t_wait_for_stage_intro();
   -- ideally this should not be a string, but I should allow closures...
   print("1_1LUA Play music");
   -- async_task("loop_bkg_music");

-- stage main
   wave_1();
   t_wait(12.5);
   wave_2();

   -- NOTE:
   -- I am using this level as a "testing" ground
   -- and I need it to get feedback on the way I design the levels
   -- and designing patterns for each 'wave' is really time consuming
   -- so these blanket waves are waves where I don't have to think much at all...
   -- and I can still pad out the play time a little longer.
   
   -- blanket_wave1()
   -- blanket_wave2()

   -- mid_boss();
   -- async_task("mid_boss_minions");

   print("1_1LUA cooldown to finish stage.");
   t_wait(10);
   wait_no_danger();
   t_complete_stage();
end
