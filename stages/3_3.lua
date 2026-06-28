engine_dofile("stages/common.lua")
engine_dofile("stages/boss3.lua")
engine_dofile("stages/subboss31.lua")
engine_dofile("stages/subboss22.lua")

-- intentionally copied from common.lua to have
-- more control over the fading visual, that doesn't affect the general version
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
      render_object_set_modulation(g_black_fader_id, 0, 0, 0, g_black_fade_t * 0.85);
      t_yield();
   end
end

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

function wave1()
   Stage1_Batflood();
   t_wait(1)
   do
      local rx = 50 - 10;
      local ry = 50 + 10;
      local e0 = Make_Enemy_Spinner_1_1_2(25, v2(45, -30), v2(0, 1), 200, 0.5, 0.5, 25, rx, ry, PROJECTILE_SPRITE_CAUSTIC_DISK);
      local e1 = Make_Enemy_Spinner_1_1_2(25, v2(play_area_width()-45, -30), v2(0, 1), 200, 0.5, 0.5, 25, rx, ry, PROJECTILE_SPRITE_CAUSTIC_DISK);
      enemy_set_visual(e0, ENTITY_SPRITE_SKULL_A1);
      enemy_set_visual(e1, ENTITY_SPRITE_SKULL_A1);
   end
   LaserChaser_Horizontal_1_2(4, 1.25);
   do
      local e0 = Make_Enemy_Burst360_1_1_2(
         15,
         v2(-10, play_area_height()/2),
         v2(50, play_area_height()/2 - 50),
         0.85,

         0.5,
         5,
         25,
         45,

         15, 30,

         v2(1, 0),
         100,
         30,

         PROJECTILE_SPRITE_CAUSTIC_STROBING,
         4
      );
      enemy_set_visual(e0, ENTITY_SPRITE_SKULL_A1);

      local e1 = Make_Enemy_Burst360_1_1_2(
         15,
         v2(play_area_width() + 10, play_area_height()/2),
         v2(play_area_width() - 50, play_area_height()/2 - 50),
         0.85,

         0.5,
         5,
         25,
         45,

         15, 30,

         v2(1, 0),
         100,
         30,

         PROJECTILE_SPRITE_CAUSTIC_STROBING,
         4
      );
      enemy_set_visual(e1, ENTITY_SPRITE_SKULL_A1);
   end
   t_wait(2);
   DramaticExplosion_SpawnSpinnerObstacle1_2_1(
      play_area_width()/2,
      play_area_height()/2,
      ENTITY_SPRITE_SKULL_A,
      12,
      30,
      PROJECTILE_SPRITE_GREEN_DISK,
      v2(0, 1),
      0
   );
   t_wait(4);
   
   DramaticExplosion_SpawnSpinnerObstacle1_2_1(
      20,
      play_area_height()-20,
      ENTITY_SPRITE_SKULL_B,
      -9,
      30,
      PROJECTILE_SPRITE_RED,
      v2(-1, -1),
      5
   );
   t_wait(4);
   
   DramaticExplosion_SpawnSpinnerObstacle1_2_1(
      play_area_width()-20,
      20,
      ENTITY_SPRITE_SKULL_B,
      -9,
      30,
      PROJECTILE_SPRITE_RED,
      v2(-1, -1),
      5
   );
   t_wait(3);
   Stage1_Batflood();
   t_wait(2.5);
   do
      for i=1,6 do
         do
            local p0 = PROJECTILE_SPRITE_RED_ELECTRIC;
            local p1 = PROJECTILE_SPRITE_RED_DISK;
            if i % 2 == 0 then
               p0 = PROJECTILE_SPRITE_BLUE_STROBING;
               p1 = PROJECTILE_SPRITE_BLUE_DISK;
            end
            local e0 = Make_Enemy_SideMoverWave1_1_1(10 + i * 35, -100, 0, 100, 1.5, 1, 0, 12, p0, p1);
            enemy_set_visual(e0, ENTITY_SPRITE_BAT_B1);
            if i % 2 == 0 then
               t_wait(0.75);
            else
               t_wait(1.0);
            end
         end
      end
   end
   -- right
   Make_BrainDead_Enemy_Popcorn1(
      20,
      v2(play_area_width() + 25, player_position_y()),
      0.086,
      7,
      -150,
      30,
      2,
      25,
      4,
      -1);
   t_wait(2.5);
   --left
   Make_BrainDead_Enemy_Popcorn1(
      32,
      v2(-30, player_position_y()),
      0.14,
      5,
      150,
      -10,
      30,
      25,
      2,
      2
   );
   t_wait(3);
   for i=1,8 do
      local e = Make_Enemy_Spinner_1_1_2(
         15, 
         v2(-15 - i*15, 20 + i * 45),
         v2(1, 0),
         80,
         2.0,
         0.0,
         45,
         5,
         5,
         PROJECTILE_SPRITE_PURPLE_DISK,
         4
         );
      -- NOTE(jerry): make new bat sprites for these things
      enemy_set_visual(e, ENTITY_SPRITE_SKULL_B1);
   end
   t_wait(2);

   do
        local e0 = Make_Enemy_SideMoverWave1_1_1(100, -100, 0, 200, 1.5, 1, 0, 8, PROJECTILE_SPRITE_RED_ELECTRIC, PROJECTILE_SPRITE_RED_DISK);
        enemy_set_visual(e0, ENTITY_SPRITE_SKULL_B);
        local e1 = Make_Enemy_SideMoverWave1_1_1(play_area_width() - 100, -100, 0, 200, 1.5, 1, 0, 4, PROJECTILE_SPRITE_GREEN_ELECTRIC, PROJECTILE_SPRITE_GREEN_DISK);
        enemy_set_visual(e1, ENTITY_SPRITE_SKULL_B);
        local e2 = Make_Enemy_SideMoverWave1_1_1(150, -100, 0, 200, 1.0, 1, 0, 8, PROJECTILE_SPRITE_RED_ELECTRIC, PROJECTILE_SPRITE_RED_DISK);
        enemy_set_visual(e2, ENTITY_SPRITE_SKULL_B);
        local e3 = Make_Enemy_SideMoverWave1_1_1(play_area_width() - 150, -100, 0, 200, 1.0, 1, 0, 4, PROJECTILE_SPRITE_GREEN_ELECTRIC, PROJECTILE_SPRITE_GREEN_DISK);
        enemy_set_visual(e3, ENTITY_SPRITE_SKULL_B);
    end
    t_wait(7);
    -- start using new enemy sprites here
    convert_all_bullets_to_score();
    do
        local e2 = Make_Enemy_SideMoverWave1_1_1(150, -100, 0, 200, 1.0, 1, 0, 8, PROJECTILE_SPRITE_RED_ELECTRIC, PROJECTILE_SPRITE_RED_DISK);
        enemy_set_visual(e2, ENTITY_SPRITE_SKULL_B);
        local e3 = Make_Enemy_SideMoverWave1_1_1(play_area_width() - 150, -100, 0, 200, 1.0, 1, 0, 4, PROJECTILE_SPRITE_GREEN_ELECTRIC, PROJECTILE_SPRITE_GREEN_DISK);
        enemy_set_visual(e3, ENTITY_SPRITE_SKULL_B);
    end
    t_wait(2.5);
end

function sub00()
    Make_BrainDead_Enemy_Popcorn1(
    38,
    v2(play_area_width()*0.5, -30),
    0.096,
    10,
    20,
    100,
    67,
    6,
    -1,
    4, ENTITY_SPRITE_SKULL_B1);
    t_wait(1.125);

   for i=1,6 do
      local e = Make_Enemy_Spinner_1_1_2(
         45, 
         v2(-15 - i*1, 0 + i * 45),
         v2(1, 0),
         80,
         1.4,
         0.0,
         45,
         5,
         5,
         PROJECTILE_SPRITE_WRM_DISK,
         4
         );
      enemy_set_visual(e, ENTITY_SPRITE_SKULL_B1);
   end
   for i=1,6 do
      local e = Make_Enemy_Spinner_1_1_2(
         45, 
         v2(play_area_width()+15 + i*35, 0 + i * 85),
         v2(-1, 0),
         80,
         2.0,
         0.0,
         45,
         5,
         5,
         PROJECTILE_SPRITE_PURPLE_DISK,
         4
         );
      enemy_set_visual(e, ENTITY_SPRITE_SKULL_B1);
   end
end

function wave2()
sub00();
   t_wait(3);

    async_task_lambda(
        function()
            Make_BrainDead_Enemy_Popcorn1(
            126,
            v2(play_area_width()*0.45, -30),
            0.096,
            8,
            20,
            100,
            67,
            6,
            -1,
            4, ENTITY_SPRITE_SKULL_B);
        end
    );
    t_wait(2.5);

    do
      local rx = 50 - 10;
      local ry = 50 + 10;
      local e0 = Make_Enemy_Spinner_1_1_2(25, v2(45, -30), v2(0, 1), 200, 0.5, 0.5, 25, rx, ry, PROJECTILE_SPRITE_PURPLE_DISK);
      local e1 = Make_Enemy_Spinner_1_1_2(25, v2(play_area_width()-45, -30), v2(0, 1), 200, 0.5, 0.5, 25, rx, ry, PROJECTILE_SPRITE_PURPLE_DISK);
      enemy_set_visual(e0, ENTITY_SPRITE_SKULL_A1);
      enemy_set_visual(e1, ENTITY_SPRITE_SKULL_A1);
   end

   t_wait(1.0);
   do
      async_task_lambda( -- NOTE: async timeline
         function()
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2,
               play_area_height()/2 - 150,
               ENTITY_SPRITE_SKULL_B1,
               10,
               20,
               PROJECTILE_SPRITE_WRM,
               v2(0, 1)
            );
         end
      )

      async_task_lambda( -- NOTE: async timeline
         function()
            t_wait(1);
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2 - 150,
               play_area_height()/2-40,
               ENTITY_SPRITE_SKULL_B,
               15,
               15,
               PROJECTILE_SPRITE_WRM,
               v2(1, 1)
            );
         end
      )

      async_task_lambda( -- NOTE: async timeline
         function()
            t_wait(1);
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2 + 150,
               play_area_height()/2 -40,
               ENTITY_SPRITE_SKULL_B,
               15,
               15,
               PROJECTILE_SPRITE_WRM,
               v2(-1, 1)
            );
         end
      )
   end
   t_wait(2);
       do
        async_task_lambda(
       function()
          for i=1, 30 do
             E0_1_1S(
                v2(0, -10),
                v2(80, 205),
                dir_to_player(v2(0, -10)), -- SHOT DIR
                0.23, 0.50, -- FIRE SPEED, FIRE DELAY
                0.90, -- TIME UNTIL ACCEL
                v2(500, 170) -- ACCEL WHERE
             );
             t_wait(0.24)
          end
       end
    )
    t_wait(1.35);
    async_task_lambda(
       function()
          for i=1, 30 do
             E0_1_1S(
                v2(play_area_width(), -10),
                v2(-100, 205),
                dir_to_player(v2(play_area_width(), -10)), -- SHOT DIR
                0.23, 0.20, -- FIRE SPEED, FIRE DELAY
                1.00, -- TIME UNTIL ACCEL
                v2(600, 100) -- ACCEL WHERE
             );
             t_wait(0.2)
          end
       end
    )
    end
end

function stage_task()
   wait_no_danger();
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_3.png", 0.15, 0, -25);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.27, 355, 15);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.147, -150, 205);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.450, 35, 0);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.800, 85, 200);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.650, 450, 200);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.750, 100, 377);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.800, -70, 200);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.552, 90, 50);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage2bkg2_1.png", 0.150, 0, 125);
   wave1();
   t_wait(1);
   wave2();
   wait_no_danger();
   t_wait(1.0);
   local subboss1 = Game_Spawn_Stage3_1_SubBoss();
   while enemy_valid(subboss1) do
     t_yield();
   end;
   wave2();
   wait_no_danger();
   t_wait(1.0);
   
   local subboss2 = Game_Spawn_Stage2_2_SubBoss();
   while enemy_valid(subboss2) do
     t_yield();
   end;
   t_wait(5);

   local finalboss = Game_Spawn_Stage_Boss();
   t_wait(2.5);
   wait_no_danger();
   t_complete_stage();
end
