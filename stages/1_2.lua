-- TODO: should be harder.
-- I mean, not too much harder, but it's pretty easy as long as you're not intimidated.
engine_dofile("stages/common.lua")

-- need this to lerp between the angles... Like a gradient
-- would probably help a lot...
-- function (e)
--    local arcs = {90, 130, 190, 200};
--    local arc_times = {1.5, 2.5, 1.4, 1.4}; -- lerp times
--    local linear_speed = 100;
--    local i = 1;
--    for i=1, 4 do
--       local arc_direction = v2_direction_from_degree(arcs[i]);
--       enemy_set_velocity(e, arc_direction[1] * linear_speed, arc_direction[2] * linear_speed);
--       t_wait(arc_times[i]);
--    end
--    print('bye');
-- end
local track = load_music("res/snds/music_1_2.ogg");


function wave1_sub1()
   -- smoothly open with a few sprayers (with some slow bullets)
   -- wait a bit
   -- have some popcorns (through the center line)
   -- then a few more sprayers.
   -- popcorn
   do
      local e0 = Make_Enemy_Burst360_1_1_2(
         10, v2(-10, -10), v2(120, 125), 1.5,

         0.12, 4,
         40, 45,

         75,
         100,

         v2(1, 1),
         45,
         45,
         
         PROJECTILE_SPRITE_HOT_PINK_ELECTRIC
         );
      enemy_set_visual(e0,
      ENTITY_SPRITE_SKULL_A);

      local e1 = Make_Enemy_Burst360_1_1_2(
         10, v2(play_area_width(), -10), v2(play_area_width()-120, 125), 1.5,

         0.12, 4,
         40, 45,

         75,
         100,

         v2(1, 1),
         45,
         45,

         PROJECTILE_SPRITE_HOT_PINK_ELECTRIC
      );
      enemy_set_visual(e1,
      ENTITY_SPRITE_SKULL_A);

      t_wait(1.25);
      -- Popcorn middle lane
      Make_BrainDead_Enemy_Popcorn1(
         20,
         v2(play_area_width() + 30, play_area_height()/2),
         0.14,
         3,
         -120,
         15,
         0,
         25,
         2,
         -1
      );

      -- Drop Bomber (popcorn)
      --
      -- an enemy with a spinning bullet shield that will release it's bullets
      -- in the last arc direction they were in when they were killed.
      --
      -- it is safer to keep these alive, but I might make them higher value
      -- since it makes the pattern "unpredictable"
      -- 
      t_wait(0.35);
      t_wait(1.15);

      -- corner occupancy
      -- and then some less threatening popcorn monsters that shoot dumb patterns
      do
         local corner_r = 30;
         local e0 = Make_Enemy_Burst360_1_1_2(
            10, v2(-10, -10), v2(corner_r, corner_r), 2.0,

            0.20, 4,
            20, 25,

            35,
            100,

            v2(1, 1),
            45,
            45,
            
            PROJECTILE_SPRITE_NEGATIVE_ELECTRIC
         );
         
         local e1 = Make_Enemy_Burst360_1_1_2(
            10, v2(play_area_width()+15, -10), v2(play_area_width()-corner_r, corner_r), 2.0,

            0.20, 4,
            20, 25,

            35,
            100,

            v2(-1, 1),
            45,
            45,
            
            PROJECTILE_SPRITE_NEGATIVE_ELECTRIC
         );

         local e2 = Make_Enemy_Burst360_1_1_2(
            10, v2(10, play_area_height()+10), v2(corner_r, play_area_height()-corner_r), 2.0,

            0.20, 4,
            20, 25,

            35,
            100,

            v2(1, -1),
            45,
            45,
            
            PROJECTILE_SPRITE_NEGATIVE_ELECTRIC
         );
         
         local e3 = Make_Enemy_Burst360_1_1_2(
            10, v2(play_area_width()+15, play_area_height()+10), v2(play_area_width()-corner_r, play_area_height()-corner_r), 2.0,

            0.20, 4,
            20, 25,

            35,
            100,

            v2(-1, -1),
            45,
            45,
            
            PROJECTILE_SPRITE_NEGATIVE_ELECTRIC
         );

         enemy_set_visual(e0, ENTITY_SPRITE_SKULL_B);
         enemy_set_visual(e1, ENTITY_SPRITE_SKULL_B);
         enemy_set_visual(e2, ENTITY_SPRITE_SKULL_B);
         enemy_set_visual(e3, ENTITY_SPRITE_SKULL_B);
      end

      Make_BrainDead_Enemy_Popcorn1(
         20,
         v2(play_area_width()/2, -20),
         0.14,
         3,
         15,
         120,
         50,
         0,
         -1,
         2
      );
   end
   t_wait(5.45);

   -- NOTE: bullets should not make particles as they will overflow the particle emitter spawner
   convert_all_bullets_to_score();
end

-- Could be harder.
function wave1_sub2()
   -- Use popcorn to funnel player into center lane.
   -- and have some enemies that try to take pot shots
   -- and a tankier enemy that comes down the center lane near the end.

   -- Time Mark: +0:18 for the popcorns
   do -- NOTE: need these to happen at the same time
      local popcorn_radius = 120;
      local popcorn_flood_width = 30;
      async_task_lambda(
         function()
            Make_BrainDead_Enemy_Popcorn1(64, v2(play_area_width()/2 - popcorn_radius, -20), 0.20, 5, 0, 90, popcorn_flood_width, 20, -1, 2);
         end
      );
      async_task_lambda(
         function()
            Make_BrainDead_Enemy_Popcorn1(64, v2(play_area_width()/2 + popcorn_radius, -20), 0.20, 5, 0, 90, popcorn_flood_width, 20, -1, 2);
         end
      );
   end
   t_wait(1.5);

   local e0 = Make_Enemy_ArcPattern2_1_1_2(
      15,
      v2(-15, 50),
      v2(30, 50),
      1.5,

      0.25,
      4,

      45,
      4,
      v2(100, 60),
      5,

      v2(-1, -1),
      50,
      10,

      PROJECTILE_SPRITE_GREEN_ELECTRIC
   )

   t_wait(1.5);
   local e1 = Make_Enemy_ArcPattern2_1_1_2(
      15,
      v2(play_area_width()+15, 100),
      v2(play_area_width()-15, 400),
      1.5,

      0.25,
      4,

      45,
      4,
      v2(-100, -70),
      5,

      v2(1, -1),
      50,
      10,

      PROJECTILE_SPRITE_GREEN_ELECTRIC
   )
   t_wait(1.5);
   local e2 = Make_Enemy_ArcPattern2_1_1_2(
      15,
      v2(play_area_width()+15, 100),
      v2(play_area_width()-15, 100),
      1.5,

      0.25,
      4,

      45,
      4,
      v2(-100, 70),
      5,

      v2(1, -1),
      50,
      10,

      PROJECTILE_SPRITE_GREEN_ELECTRIC
   )
   enemy_set_visual(e0, ENTITY_SPRITE_SKULL_B);
   enemy_set_visual(e1, ENTITY_SPRITE_SKULL_A);
   enemy_set_visual(e2, ENTITY_SPRITE_SKULL_A);
   t_wait(2.0);

   -- two tunneling bigger enemies
   do
      for i=1,6  do
         do
            local e = enemy_new();
            enemy_set_hp(e, 60); -- requires focus to break
            enemy_set_position(e, play_area_width()/2 - 25, -30);
            enemy_set_scale(e, 20, 20);
            enemy_move_linear(e, v2(0, 1), 120);
            enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
            enemy_set_visual_scale(e, 2, 2);
         end
         do
            local e = enemy_new();
            enemy_set_hp(e, 60); -- requires focus to break
            enemy_set_position(e, play_area_width()/2 + 25, -30);
            enemy_set_scale(e, 20, 20);
            enemy_move_linear(e, v2(0, 1), 120);
            enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
            enemy_set_visual_scale(e, 2, 2);
         end
         t_wait(1.25);
      end
   end
end

function wave1_sub3()
   -- new enemy type, crazy spinster
   -- have two of them run along the screen of it running across the screen

   -- along with a few regular enemies that shoot a lingering firework pattern
   -- then a generous amount of random popcorns

   -- Theoretically : 0:12
   async_task_lambda(
      function () -- CONCERN: might be visually hard to see?
         do
            local radius = 40;
            for i=1, 2 do
               local rx = 50 - i*10;
               local ry = 25 + i*10;
               local e0 = Make_Enemy_Spinner_1_1_2(25, v2(-45, radius), v2(1, 0), 120, 1.0, 1.00, 30, rx, ry, PROJECTILE_SPRITE_SPARKLING_STAR);
               local e1 = Make_Enemy_Spinner_1_1_2(25, v2(-45, play_area_height() - radius), v2(1, 0), 120, 1.0, 1.00, 30, rx, ry, PROJECTILE_SPRITE_SPARKLING_STAR);
               enemy_set_visual(e0, ENTITY_SPRITE_SKULL_B);
               enemy_set_visual(e1, ENTITY_SPRITE_SKULL_B);
               t_wait(4.0);
            end
            -- TODO: add trail. I want these to be shooting stars. literally.
         end
      end
   )
   -- flood with popcorn from left and right
   -- TODO:
   t_wait(5.0);
   do
      local e0 =Make_Enemy_Burst360_1_1_2(
         35, v2(-10, -10), v2(play_area_width()/2, 30), 0.5,

         0.7,
         3,

         40, 45,

         50,
         70,

         v2(0, 1),
         45,
         45,
         
         PROJECTILE_SPRITE_HOT_PINK_ELECTRIC
      );

      t_wait(0.5);
      local e1 = Make_Enemy_Burst360_1_1_2(
         35, v2(-10, play_area_height() + 40), v2(play_area_width()/2 - 40, 80), 0.5,

         0.7,
         3,

         40, 45,

         50,
         70,

         v2(-1, 1),
         70,
         45,
         
         PROJECTILE_SPRITE_GREEN_DISK
      );

      local e2 = Make_Enemy_Burst360_1_1_2(
         35, v2(play_area_width()-10, play_area_height() + 40), v2(play_area_width()/2 + 40, 80), 0.5,

         0.7,
         3,

         40, 45,

         50,
         70,

         v2(1, 1),
         70,
         45,
         
         PROJECTILE_SPRITE_BLUE_DISK
      );

      enemy_set_visual(e0, ENTITY_SPRITE_SKULL_B);
      enemy_set_visual(e1, ENTITY_SPRITE_SKULL_A);
      enemy_set_visual(e2, ENTITY_SPRITE_SKULL_A);

      t_wait(1);
      do -- NOTE: need these to happen at the same time
         local popcorn_radius = 250;
         local popcorn_flood_width = 40;
         async_task_lambda(
            function()
               Make_BrainDead_Enemy_Popcorn1(8, v2(play_area_width()/2 - popcorn_radius, -20), 0.20, 5, 20, 120, popcorn_flood_width, 20, -1, 2);
            end
         );
         async_task_lambda(
            function()
               Make_BrainDead_Enemy_Popcorn1(8, v2(play_area_width()/2 + popcorn_radius, -20), 0.20, 5, -20, 120, popcorn_flood_width, 20, -1, 2);
            end
         );
      end
   end
   for i=1,5 do
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
         PROJECTILE_SPRITE_BLUE_DISK,
         4
         );
      -- NOTE(jerry): make new bat sprites for these things
      enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
   end
   t_wait(9);
   convert_all_bullets_to_score();
   t_wait(3);

   -- Play thunder sound or something
   -- and start the rain!

   -- Time Mark : 0:18
   MainBoss1_RainCloud_Attack1(1337, 12);
   t_wait(13.5);
   convert_all_bullets_to_score();
   t_wait(2);
   MainBoss1_RainCloud_Attack2(4938, 8);
   t_wait(10);
end

function wave_1()
   print("wave1 sub1");
   wave1_sub1();

   -- Time Mark : 0:18
   -- Bullets should've just been cleared at this point

   print("wave1 sub2");
   wave1_sub2();
   print("wave1 sub3");
   t_wait(4);

   -- Time Mark:  0:22
   -- screen should be empty again
   wave1_sub3()

   -- Time Mark : 1:10
   do -- NOTE: need these to happen at the same time
      local popcorn_radius = 200;
      local popcorn_flood_width = 50;
      async_task_lambda(
         function()
            local enemies = Make_BrainDead_Enemy_Popcorn1(16, v2(play_area_width()/2 - popcorn_radius, -20), 0.20, 5, 30, 120, popcorn_flood_width, 45, -1, -1);
            for i,e in ipairs(enemies) do
               Enemy_AddExplodeOnDeathBehavior(e, 60, 0.1, 1.0);
            end
         end
      );
      for i=1,5 do
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
            PROJECTILE_SPRITE_BLUE_DISK,
            4
            );
         -- NOTE(jerry): make new bat sprites for these things
         enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
      end
      async_task_lambda(
         function()
            local enemies = Make_BrainDead_Enemy_Popcorn1(16, v2(play_area_width()/2 + popcorn_radius, -20), 0.20, 5, -30, 120, popcorn_flood_width, 45, -1, -1);
            for i,e in ipairs(enemies) do
               Enemy_AddExplodeOnDeathBehavior(e, 60, 0.1, 1.0);
            end
         end
      );
   end
   t_wait(10);
end

function wave_2()
   async_task_lambda(
      function () -- CONCERN: might be visually hard to see?
         do
            do
               local rx = 50 - 10;
               local ry = 50 + 10;
               local e0 = Make_Enemy_Spinner_1_1_2(25, v2(45, -30), v2(0, 1), 200, 0.5, 0.5, 25, rx, ry, PROJECTILE_SPRITE_GREEN_DISK);
               local e1 = Make_Enemy_Spinner_1_1_2(25, v2(play_area_width()-45, -30), v2(0, 1), 200, 0.5, 0.5, 25, rx, ry, PROJECTILE_SPRITE_RED_DISK);
               enemy_set_visual(e0, ENTITY_SPRITE_SKULL_B);
               enemy_set_visual(e1, ENTITY_SPRITE_SKULL_B);
               t_wait(1.0);
            end
            t_wait(1.5);

            -- introduce first laser.

            laser_hazard_new(player_position_y(), 15, 0, 0.05, 0.5);
            do
               local e0 = Make_Enemy_Burst360_1_1_2(
                  10, v2(-10, -10), v2(120, 125), 1.5,

                  0.12, 4,
                  40, 45,

                  75,
                  100,

                  v2(1, 1),
                  45,
                  45,
                  
                  PROJECTILE_SPRITE_NEGATIVE_DISK
               );

               local e1 = Make_Enemy_Burst360_1_1_2(
                  10, v2(play_area_width(), -10), v2(play_area_width()-120, 125), 1.5,

                  0.12, 4,
                  40, 45,

                  75,
                  100,

                  v2(1, 1),
                  45,
                  45,

                  PROJECTILE_SPRITE_NEGATIVE_DISK
               );

               enemy_set_visual(e0, ENTITY_SPRITE_BAT_B);
               enemy_set_visual(e1, ENTITY_SPRITE_BAT_B);
            end

            -- bottom left
            Make_BrainDead_Enemy_Popcorn1(
               32,
               v2(-30, play_area_height()/2),
               0.14,
               5,
               120,
               -100,
               30,
               25,
               2,
               2
            );
         end
      end
   )
   t_wait(12.5);
   convert_all_enemies_to_score();
   convert_all_bullets_to_score();
   t_wait(2);
end

function preboss_wave()
   -- laser frenzy
   -- Curtain Close
   do
      for i=1,3 do
         local offset = i * 45;
         laser_hazard_new(offset, 10, 0, 0.05, 1);
         laser_hazard_new(play_area_height() - offset, 10, 0, 0.05, 1);
         t_wait(0.4);
      end
      t_wait(1.0);
      for i=1,3 do
         local offset = i * 45;
         laser_hazard_new(offset, 10, 1, 0.05, 1);
         laser_hazard_new(play_area_width() - offset, 10, 1, 0.05, 1);
         t_wait(0.4);
      end
      t_wait(4);
      LaserChaser_Vertical_1_2(8, 1.5);
      t_wait(1.2);
   end
   MainBoss1_RainCloud_Attack2(1997, 3.5)
   t_wait(6.0);
   convert_all_bullets_to_score()
   t_wait(6.0);
   Stage1_Batflood();
   t_wait(12.0); -- TODO check if they're actually off the screen.
   kill_all_enemies();
   -- Dialogue point with midboss!
   -- fight the midboss! EXPECT 30 seconds
end

function stage_task()
   play_area_set_edge_behavior(PLAY_AREA_EDGE_TOP, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_BOTTOM, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_LEFT, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_RIGHT, PLAY_AREA_EDGE_BLOCKING);
   play_music(track);

   -- setup stage background
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg2_0.png", 0.45, 0, 0);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg2_nebule.png", 0.287, 0, 120);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg2_nebule.png", 0.187, 450, 350);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star0.png", 0.357, 225, 125);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star1.png", 0.147, -150, 245);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.287, 305, 300);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.207, -100, 225);

   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.127, 100, 15);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.257, -350, 255);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg2_1_2.png", 0.150, 0, 125);
   play_area_notify_current_border_status();
   -- end stage setup background

   t_wait(2);
   -- print("wave 1");
   wave_1();
   -- print("wave 2");
   wave_2();
   preboss_wave();
   print("wait finish");
   t_wait(1);

   -- last
   Stage1_Batflood();
   do
      local e0 =Make_Enemy_Burst360_1_1_2(
         35, v2(-10, play_area_height() + 40), v2(play_area_width()/2 - 40, 80), 0.5,

         0.7,
         5,

         40, 85,

         50,
         70,

         v2(-1, 1),
         70,
         45,
         
         PROJECTILE_SPRITE_GREEN_DISK
      );

      local e1 = Make_Enemy_Burst360_1_1_2(
         35, v2(play_area_width()-10, play_area_height() + 40), v2(play_area_width()/2 + 40, 80), 0.5,

         0.7,
         5,

         40, 95,

         60,
         70,

         v2(1, 1),
         70,
         45,
         
         PROJECTILE_SPRITE_BLUE_DISK
      );
      enemy_set_visual(e0, ENTITY_SPRITE_BAT_B);
      enemy_set_visual(e1, ENTITY_SPRITE_BAT_B);

      t_wait(1);
      local e2 = Make_Enemy_Burst360_1_1_2(
         35, v2(-10, play_area_height() + 40), v2(play_area_width()/2 - 40, 200), 0.5,

         0.7,
         5,

         40, 105,

         50,
         70,

         v2(-1, 1),
         70,
         45,
         
         PROJECTILE_SPRITE_GREEN_DISK
      );

      local e3 = Make_Enemy_Burst360_1_1_2(
         35, v2(play_area_width()-10, play_area_height() + 40), v2(play_area_width()/2 + 40, 200), 0.5,

         0.7,
         5,

         40, 105,

         35,
         70,

         v2(1, 1),
         70,
         45,
         
         PROJECTILE_SPRITE_BLUE_DISK
      );

      enemy_set_visual(e2, ENTITY_SPRITE_BAT_B);
      enemy_set_visual(e3, ENTITY_SPRITE_BAT_B);
   end
   t_wait(4.5);
   LaserChaser_Horizontal_1_2(8, 1.5);
   LaserChaser_Horizontal_1_2(4, 0.75);
   LaserChaser_Vertical_1_2(4, 2.5);
   LaserChaser_Vertical_1_2(4, 0.75);

   wait_no_danger();
   t_complete_stage();
end
