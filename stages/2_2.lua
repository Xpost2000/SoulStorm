engine_dofile("stages/common.lua")

-- move to common.lua
-- TODO SKULL C
function wave1()
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

   t_wait(6);
   do
      for i=1,4 do
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

      t_wait(8);
      do
         Make_BrainDead_Enemy_Popcorn1(
            45,
            v2(play_area_width() + 40, play_area_height()/2),
            0.065,
            5,
            -150,
            -5,
            5,
            5,
            2,
            -1,
            ENTITY_SPRITE_SKULL_A
         );
      end
      t_wait(10);
      do
         Make_BrainDead_Enemy_Popcorn1(
            45,
            v2(play_area_width() + 40, play_area_height()/2),
            0.065,
            5,
            -150,
            -5,
            5,
            5,
            2,
            -1,
            ENTITY_SPRITE_BAT_B
         );
      end
      t_wait(4)
      convert_all_bullets_to_score();
   end
end

function wave2()
   if true then
      -- new enemy type, I need a new sprite
      for i=1,4 do
         local e = Make_Enemy_Spinner_1_1_2(
            15, 
            v2(-15 - i*15, 20 + i * 45),
            v2(1, 0),
            120,
            2.0,
            0.0,
            100,
            5,
            5,
            PROJECTILE_SPRITE_GREEN_DISK,
            4
            );
         enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
      end
      for i=1,4 do
         local e = Make_Enemy_Spinner_1_1_2(
            15, 
            v2(play_area_width() + i*15, 20 + i * 45),
            v2(-1, 0),
            120,
            2.0,
            0.0,
            100,
            5,
            5,
            PROJECTILE_SPRITE_GREEN_DISK,
            4
            );
         enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
      end
      t_wait(3)
      for i=1,6 do
         local e = Make_Enemy_Spinner_1_1_2(
            15, 
            v2(-15 - i*15, 20 + i * 45),
            v2(1, 0),
            120,
            1.0,
            0.0,
            50,
            5,
            5,
            PROJECTILE_SPRITE_NEGATIVE_DISK,
            4
            );
         enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
      end
      for i=1,6 do
         local e = Make_Enemy_Spinner_1_1_2(
            15, 
            v2(play_area_width() + i*15, 20 + i * 45),
            v2(-1, 0),
            120,
            1.0,
            0.0,
            50,
            5,
            5,
            PROJECTILE_SPRITE_NEGATIVE_DISK,
            4
            );
         enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
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
      LaserChaser_Horizontal_1_2(4, 1.25);
      for i=1,3 do
         local e = Make_Enemy_Spinner_1_1_2(
            15, 
            v2(-15 - i*15, 20 + i * 45),
            v2(1, 0),
            120,
            1.0,
            0.0,
            50,
            5,
            5,
            PROJECTILE_SPRITE_RED_DISK,
            4
            );
         enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
      end
      for i=1,3 do
         local e = Make_Enemy_Spinner_1_1_2(
            15, 
            v2(play_area_width() + i*15, 20 + i * 45),
            v2(-1, 0),
            120,
            1.0,
            0.0,
            50,
            5,
            5,
            PROJECTILE_SPRITE_RED_DISK,
            4
            );
         enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
      end
      do
         local e0 = Make_Enemy_SideMoverWave1_1_1(100, -100, 0, 200, 1.5, 1, 0, 8, PROJECTILE_SPRITE_RED_ELECTRIC, PROJECTILE_SPRITE_RED_DISK);
         enemy_set_visual(e0, ENTITY_SPRITE_SKULL_B);
         local e1 = Make_Enemy_SideMoverWave1_1_1(play_area_width() - 100, -100, 0, 200, 1.5, 1, 0, 4, PROJECTILE_SPRITE_GREEN_ELECTRIC, PROJECTILE_SPRITE_GREEN_DISK);
         enemy_set_visual(e1, ENTITY_SPRITE_SKULL_B);
      end
      t_wait(2.5);
      LaserChaser_Vertical_1_2(4, 1.25);
   end

   t_wait(5);
   disable_grazing();
   do
      local e0 = Make_Enemy_Helix_Turret_2_2(
      25,
      v2(0, -30),
      v2(100, 40),
      0.5,

      0.5,
      8,
      0.45,
      v2(0, 180),

      v2(1, 1),
      50,
      25,

      8,
      5, -- bullets per helix

      -- should use new sprite
      PROJECTILE_SPRITE_GREEN,  -- helix 1
      PROJECTILE_SPRITE_RED,  -- helix 2
      4
      )
      enemy_set_visual(e0, ENTITY_SPRITE_SKULL_B);
      local e1 = Make_Enemy_Helix_Turret_2_2(
         25,
         v2(0, -30),
         v2(play_area_width()-100, 40),
         0.5,
   
         0.5,
         8,
         0.45,
         v2(0, 180),
   
         v2(1, 1),
         50,
         25,
   
         8,
         5, -- bullets per helix
   
         -- should use new sprite
         PROJECTILE_SPRITE_HOT_PINK,  -- helix 1
         PROJECTILE_SPRITE_BLUE_STROBING,  -- helix 2
         4
         )
         enemy_set_visual(e1, ENTITY_SPRITE_SKULL_B);
   end
   -- NOTE lifted from 2-1
   -- also use new sprites here
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

         PROJECTILE_SPRITE_GREEN_STROBING,
         4
      );
      enemy_set_visual(e0, ENTITY_SPRITE_SKULL_A);

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

         PROJECTILE_SPRITE_GREEN_STROBING,
         4
      );
      enemy_set_visual(e1, ENTITY_SPRITE_SKULL_A);
      t_wait(1);
      local e2 = Make_Enemy_Spinster_1_1(
         play_area_width()/2,
         -20,
         0, 
         40, 
         1.0, 0, 2, PROJECTILE_SPRITE_RED, 0,
         15, 20, 15
      );
      enemy_set_visual(e2, ENTITY_SPRITE_BAT_B);
      t_wait(4);

      local e3 = Make_Enemy_Burst360_1_1_2_EX(
         15,
         v2(-10, play_area_height()/2),
         v2(50, play_area_height()/2 +10),
         0.85,

         0.5,
         3,

         35,
         125,

         5, 125,

         v2(1, 0),
         100,
         30,

         1,
         PROJECTILE_SPRITE_NEGATIVE,1
      );
      enemy_set_visual(e3, ENTITY_SPRITE_SKULL_B);

      local e4 = Make_Enemy_Burst360_1_1_2_EX(
         750,
         v2(play_area_width() + 10, play_area_height()/2),
         v2(play_area_width() - 50, play_area_height()/2 +10),
         0.85,

         0.65,
         3,

         25,
         125,

         5, 120,

         v2(1, 0),
         180,
         30,

         1,
         PROJECTILE_SPRITE_NEGATIVE,1
      );
      enemy_set_visual(e4, ENTITY_SPRITE_SKULL_B);
      t_wait(5);
      local e5 = Make_Enemy_Burst360_1_1_2_EX(
         750,
         v2(-10, play_area_height()/2),
         v2(50, play_area_height()/2 - 50),
         0.85,

         0.65,
         2,

         35,
         90,

         5, 125,

         v2(1, 0),
         180,
         30,

         1,
         PROJECTILE_SPRITE_RED
      );
      enemy_set_visual(e5, ENTITY_SPRITE_SKULL_B);

      local e6 = Make_Enemy_Burst360_1_1_2_EX(
         750,
         v2(play_area_width() + 10, play_area_height()/2),
         v2(play_area_width() - 50, play_area_height()/2 - 50),
         0.85,

         0.5,
         2,

         25,
         90,

         5, 120,

         v2(1, 0),
         180,
         30,

         1,
         PROJECTILE_SPRITE_RED
      );
      enemy_set_visual(e6, ENTITY_SPRITE_SKULL_B);
      t_wait(15);
      convert_all_bullets_to_score();
   end
   enable_grazing();
   t_wait(4);
   do
      async_task_lambda( -- NOTE: async timeline
         function()
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2,
               play_area_height()/2 - 150,
               ENTITY_SPRITE_SKULL_B,
               10,
               30,
               PROJECTILE_SPRITE_RED_DISK,
               v2(0, 1),
               4
            );
         end
      )

      async_task_lambda( -- NOTE: async timeline
      function()
         DramaticExplosion_SpawnSpinnerObstacle1_2_1(
            play_area_width()/2+50,
            play_area_height()/2 - 90,
            ENTITY_SPRITE_SKULL_B,
            10,
            30,
            PROJECTILE_SPRITE_GREEN_DISK,
            v2(0, 1),
            4
         );
      end
      )

      
      async_task_lambda( -- NOTE: async timeline
      function()
         DramaticExplosion_SpawnSpinnerObstacle1_2_1(
            play_area_width()/2-50,
            play_area_height()/2 - 90,
            ENTITY_SPRITE_SKULL_B1,
            10,
            30,
            PROJECTILE_SPRITE_GREEN_DISK,
            v2(0, 1),
            4
         );
      end
      )

      async_task_lambda( -- NOTE: async timeline
         function()
            t_wait(1);
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2 - 150,
               play_area_height()/2,
               ENTITY_SPRITE_SKULL_B1,
               15,
               25,
               PROJECTILE_SPRITE_RED_DISK,
               v2(1, 1),
               4
            );
         end
      )

      async_task_lambda( -- NOTE: async timeline
         function()
            t_wait(1);
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2 + 150,
               play_area_height()/2,
               ENTITY_SPRITE_SKULL_B1,
               15,
               25,
               PROJECTILE_SPRITE_RED_DISK,
               v2(-1, 1),
               4
            );
         end
      )

      t_wait(10)
      do
         Make_BrainDead_Enemy_Popcorn1(
            10,
            v2(play_area_width() + 10, play_area_height()/2 - 30),
            0.086,
            7,
            -180,
            30,
            2,
            25,
            4,
            -1);
         t_wait(0.125);
         Make_BrainDead_Enemy_Popcorn1(
            10,
            v2(-10, play_area_height()/2 - 50),
            0.086,
            7,
            180,
            30,
            2,
            25,
            4,
            -1);
         t_wait(0.325);
      end
      t_wait(10)
      do
         Make_BrainDead_Enemy_Popcorn1(
            10,
            v2(play_area_width() + 10, play_area_height()/2 - 30),
            0.086,
            7,
            -180,
            30,
            2,
            25,
            4,
            -1);
         t_wait(0.125);
         Make_BrainDead_Enemy_Popcorn1(
            10,
            v2(-10, play_area_height()/2 - 50),
            0.086,
            7,
            180,
            30,
            2,
            25,
            4,
            -1);
         t_wait(0.325);
      end
   end
   t_wait(4);
   convert_all_bullets_to_score();
   async_task_lambda(
      function ()
         t_wait(4.5);
         for i=1,4 do
            local v = PROJECTILE_SPRITE_GREEN_DISK;
            if i % 2 == 0 then
               v = PROJECTILE_SPRITE_RED_DISK;
            end
            local e = Make_Enemy_Spinner_1_1_2(
               15, 
               v2(play_area_width() + i*15, 20 + i * 45),
               v2(-1, 0),
               120,
               0.33,
               0.0,
               55,
               5,
               5,
               v,
               8
               );
            enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
         end
      end
   )
end

function wave4_2_1()
   do
      DramaticExplosion_SpawnShotgunSpread(
         play_area_width() * 0.25, 20, ENTITY_SPRITE_SKULL_B,
         50, 6, 10, PROJECTILE_SPRITE_PURPLE_STROBING, 1.5, 0.55, v2(0, 1), 90,
         10
      );
      DramaticExplosion_SpawnShotgunSpread(
         play_area_width() * 0.65, 20, ENTITY_SPRITE_SKULL_B1,
         50, 8, 10, PROJECTILE_SPRITE_PURPLE_STROBING, 1.5, 0.55, v2(0, 1), 90,
         10
      );

      DramaticExplosion_SpawnShotgunSpread(
         play_area_width() * 0.35, 10, ENTITY_SPRITE_SKULL_B,
         50, 8, 10, PROJECTILE_SPRITE_PURPLE_STROBING, 1.5, 0.55, v2(0.2, 1), 55,
         15
      );
      DramaticExplosion_SpawnShotgunSpread(
         play_area_width() * 0.45, 10, ENTITY_SPRITE_SKULL_B1,
         50, 6, 10, PROJECTILE_SPRITE_PURPLE_STROBING, 1.5, 0.55, v2(-0.2, 1), 55,
         15
      );

      Make_BrainDead_Enemy_Popcorn1(
         25,
         v2(play_area_width()*0.7, -30),
         0.086,
         12,
         10,
         220,
         35,
         6,
         -1,
         4);
      t_wait(0.125);
      Make_BrainDead_Enemy_Popcorn1(
         25,
         v2(play_area_width()*0.4, -30),
         0.086,
         12,
         10,
         220,
         35,
         6,
         -1,
         4);
      t_wait(0.125);
      Make_BrainDead_Enemy_Popcorn1(
         32,
         v2(play_area_width()*0.2, -30),
         0.086,
         12,
         10,
         220,
         35,
         6,
         -1,
         4);
      t_wait(0.325);
   end
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
         PROJECTILE_SPRITE_WRM_DISK,
         4
         );
      -- NOTE(jerry): make new bat sprites for these things
      enemy_set_visual(e, ENTITY_SPRITE_SKULL_B1);
   end
   t_wait(2);
end

-- copied and modified a little bit.
function wave3_2_1()
   -- figure out what time this is
   -- Add a few of the level 1 Home attackers
   local e0 = Make_Enemy_SideMoverWave1_1_1(100, -100, 0, 200, 1.5, 1, 0, 8, PROJECTILE_SPRITE_RED_ELECTRIC, PROJECTILE_SPRITE_RED_DISK);
   enemy_set_visual(e0, ENTITY_SPRITE_SKULL_B);
   local e3 = Make_Enemy_SideMoverWave1_1_1(play_area_width() - 150, -100, 0, 200, 1.0, 1, 0, 4, PROJECTILE_SPRITE_GREEN_ELECTRIC, PROJECTILE_SPRITE_GREEN_DISK);
   enemy_set_visual(e3, ENTITY_SPRITE_SKULL_B);
   -- some popcorns,
   t_wait(2);

   -- TODO replace with dog flood
   Stage1_Batflood();
   t_wait(4);
   -- Add a spinster (stage 1 style)
   local e4 = Make_Enemy_Spinster_1_1(play_area_width()/2,
                           -20,
                           0, 40, 1.0, 0, 3, PROJECTILE_SPRITE_PURPLE_STROBING, 0,
                           15, 20, 15
   );
   enemy_set_visual(e4, ENTITY_SPRITE_SKULL_A);
   t_wait(1.5);
   local e5 = Make_Enemy_Spinster_1_1(-20,
                           play_area_height()/2,
                           40, 0, 1.0, 0, 3, PROJECTILE_SPRITE_WRM_STROBING, -90,
                           15, 20, 25
   );
   enemy_set_visual(e5, ENTITY_SPRITE_SKULL_A);
   local e6 = Make_Enemy_Spinster_1_1(play_area_width()+20,
                           play_area_height()/2,
                           -20, 0, 1.0, 0, 3, PROJECTILE_SPRITE_PURPLE_STROBING, 90,
                           15, 20, 25
   );
   enemy_set_visual(e6, ENTITY_SPRITE_SKULL_A);
   -- Add a nearly impossible scenario (dodge by staying in the center)
   t_wait(2);

   -- harder version of wave2
   disable_grazing();
   disable_bullet_to_points();
   do
      async_task_lambda( -- NOTE: async timeline
         function()
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2,
               play_area_height()/2 - 150,
               ENTITY_SPRITE_SKULL_B,
               10,
               30,
               PROJECTILE_SPRITE_HOT_PINK_DISK,
               v2(0, 1)
            );
         end
      )

      async_task_lambda( -- NOTE: async timeline
         function()
            t_wait(1);
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2 - 150,
               play_area_height()/2,
               ENTITY_SPRITE_SKULL_A,
               15,
               36,
               PROJECTILE_SPRITE_HOT_PINK_DISK,
               v2(1, 1)
            );
         end
      )

      async_task_lambda( -- NOTE: async timeline
         function()
            t_wait(1);
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2 + 150,
               play_area_height()/2,
               ENTITY_SPRITE_SKULL_A,
               15,
               36,
               PROJECTILE_SPRITE_HOT_PINK_DISK,
               v2(-1, 1)
            );
         end
      )
   end

   t_wait(3.5);
   do
      local e0 = Make_Enemy_Burst360_1_1_2(
         15,
         v2(-10, play_area_height()/2),
         v2(100, play_area_height()/2 - 50),
         0.85,

         0.5,
         2,
         25,
         45,

         15, 45,

         v2(1, 0),
         100,
         30,

         PROJECTILE_SPRITE_PURPLE_STROBING
      );
      enemy_set_visual(e0, ENTITY_SPRITE_BAT_B);

      local e1 = Make_Enemy_Burst360_1_1_2(
         15,
         v2(play_area_width() + 10, play_area_height()/2),
         v2(play_area_width()/2 + 100, play_area_height()/2 - 50),
         0.85,

         0.5,
         2,
         25,
         45,

         15, 45,

         v2(1, 0),
         100,
         30,

         PROJECTILE_SPRITE_CAUSTIC_STROBING
      );
      enemy_set_visual(e1, ENTITY_SPRITE_BAT_B);
   end
   t_wait(3.5);
   do
      Make_BrainDead_Enemy_Popcorn1(
         10,
         v2(play_area_width() + 10, play_area_height()/2 - 30),
         0.086,
         7,
         -180,
         30,
         2,
         25,
         4,
         -1);
      t_wait(0.125);
      Make_BrainDead_Enemy_Popcorn1(
         10,
         v2(-10, play_area_height()/2 - 50),
         0.086,
         7,
         180,
         30,
         2,
         25,
         4,
         -1);
      t_wait(0.325);
   end

   async_task_lambda(
      function()
         for i=1, 30 do
            E0_1_1S(
               v2(0, -10),
               v2(150, 205),
               dir_to_player(v2(0, -10)), -- SHOT DIR
               0.23, 0.50, -- FIRE SPEED, FIRE DELAY
               0.90, -- TIME UNTIL ACCEL
               v2(400, 170) -- ACCEL WHERE
            );
            t_wait(0.13)
         end
      end
   )
   t_wait(1.35);
   async_task_lambda(
      function()
         for i=1, 30 do
            E0_1_1S(
               v2(play_area_width(), -10),
               v2(-150, 205),
               dir_to_player(v2(play_area_width(), -10)), -- SHOT DIR
               0.23, 0.50, -- FIRE SPEED, FIRE DELAY
               1.00, -- TIME UNTIL ACCEL
               v2(400, 100) -- ACCEL WHERE
            );
            t_wait(0.17)
         end
      end
   )

   -- This might be pretty hard.
   enable_grazing();
   enable_bullet_to_points();

   -- Add an enemy flood wave (should be dog sprites [TODO: modify dog hero sprite])
   -- and use them as the new enemies.
   Stage1_Batflood();
   -- again to padout the spinster times
   t_wait(2);
   do
      async_task_lambda( -- NOTE: async timeline
         function()
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2,
               play_area_height()/2 - 150,
               ENTITY_SPRITE_SKULL_B,
               10,
               30,
               PROJECTILE_SPRITE_HOT_PINK_DISK,
               v2(0, 1)
            );
         end
      )

      async_task_lambda( -- NOTE: async timeline
         function()
            t_wait(1);
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2 - 150,
               play_area_height()/2,
               ENTITY_SPRITE_SKULL_A,
               15,
               36,
               PROJECTILE_SPRITE_HOT_PINK_DISK,
               v2(1, 1)
            );
         end
      )

      async_task_lambda( -- NOTE: async timeline
         function()
            t_wait(1);
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2 + 150,
               play_area_height()/2,
               ENTITY_SPRITE_SKULL_A,
               15,
               36,
               PROJECTILE_SPRITE_HOT_PINK_DISK,
               v2(-1, 1)
            );
         end
      )
   end

   t_wait(3.5);
   do
      local e0 = Make_Enemy_Burst360_1_1_2(
         15,
         v2(-10, play_area_height()/2),
         v2(100, play_area_height()/2 - 50),
         0.85,

         0.5,
         2,
         25,
         45,

         15, 45,

         v2(1, 0),
         100,
         30,

         PROJECTILE_SPRITE_PURPLE_STROBING
      );
      enemy_set_visual(e0, ENTITY_SPRITE_BAT_B);

      local e1 = Make_Enemy_Burst360_1_1_2(
         15,
         v2(play_area_width() + 10, play_area_height()/2),
         v2(play_area_width()/2 + 100, play_area_height()/2 - 50),
         0.85,

         0.5,
         2,
         25,
         45,

         15, 45,

         v2(1, 0),
         100,
         30,

         PROJECTILE_SPRITE_CAUSTIC_STROBING
      );
      enemy_set_visual(e1, ENTITY_SPRITE_BAT_B);
   end
   t_wait(3.5);
   do
      Make_BrainDead_Enemy_Popcorn1(
         10,
         v2(play_area_width() + 10, play_area_height()/2 - 30),
         0.086,
         7,
         -180,
         30,
         2,
         25,
         4,
         -1);
      t_wait(0.125);
      Make_BrainDead_Enemy_Popcorn1(
         10,
         v2(-10, play_area_height()/2 - 50),
         0.086,
         7,
         180,
         30,
         2,
         25,
         4,
         -1);
      t_wait(0.325);
   end

   async_task_lambda(
      function()
         for i=1, 30 do
            E0_1_1S(
               v2(0, -10),
               v2(150, 205),
               dir_to_player(v2(0, -10)), -- SHOT DIR
               0.23, 0.50, -- FIRE SPEED, FIRE DELAY
               0.90, -- TIME UNTIL ACCEL
               v2(400, 170) -- ACCEL WHERE
            );
            t_wait(0.13)
         end
      end
   )
   t_wait(1.35);
   async_task_lambda(
      function()
         for i=1, 30 do
            E0_1_1S(
               v2(play_area_width(), -10),
               v2(-150, 205),
               dir_to_player(v2(play_area_width(), -10)), -- SHOT DIR
               0.23, 0.50, -- FIRE SPEED, FIRE DELAY
               1.00, -- TIME UNTIL ACCEL
               v2(400, 100) -- ACCEL WHERE
            );
            t_wait(0.17)
         end
      end
   )
end

function stage_task()
   play_area_set_edge_behavior(PLAY_AREA_EDGE_TOP, PLAY_AREA_EDGE_DEADLY);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_BOTTOM, PLAY_AREA_EDGE_DEADLY);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_LEFT, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_RIGHT, PLAY_AREA_EDGE_BLOCKING);
   -- setup stage background
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage2bkg_0.png", 0.53, 0, 0); -- TODO come up with new design

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star0.png", 0.357, 225, 125);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star1.png", 0.147, -150, 245);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.287, 305, 300);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.207, -100, 225);

   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.27, 100, 65);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.37, 500, 200);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage2bkg2_1.png", 0.150, 0, 125);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage2bkg2_1_2.png", 0.150, 0, 125);
   play_area_notify_current_border_status();
   t_wait(3);
   wave1();
   t_wait(1);
   wave2();
   t_wait(5);
   wave4_2_1();
   t_wait(3);
   wave3_2_1();
   t_wait(6);
   convert_all_bullets_to_score();
   wait_no_danger();
   t_complete_stage();
end
