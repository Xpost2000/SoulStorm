engine_dofile("stages/common.lua")

-- move to common.lua
function Make_Enemy_Helix_Turret_2_2(
   hp,
   initial_position,
   target_position,
   target_position_lerp_t,

   fire_delay,
   burst_count,
   burst_delay,
   bullet_velocity,

   exit_direction,
   exit_velocity,
   exit_acceleration,

   helix_height,
   helix_length,

   bullet_visual,
   bullet_visual1,
   trailcount
)
   local e = enemy_new();
   enemy_set_hp(e, hp);
   enemy_set_position(e, initial_position[1], initial_position[2]);
   local tcount = trailcount or 0;
   exit_direction = v2_normalized(exit_direction);
   local bullet_direction = v2_normalized(bullet_velocity);
   local bullet_direction_perp = v2(-bullet_direction[2], bullet_direction[1]);

   enemy_task_lambda(
      e,
      function(e)
         -- cannot use asymptopic movement yet
         enemy_linear_move_to(e, target_position[1], target_position[2], target_position_lerp_t);
         t_wait(fire_delay);

         -- fire helix in here.
         -- NOTE: this helix is static, should be animated
         -- though...
         local ex = enemy_position_x(e);
         local ey = enemy_position_y(e);
         
         for burst=0,burst_count do
            for helixi=0,helix_length do
               local cyclepercent = (helixi/helix_length) * 2*3.141592654;
               local y1 = math.sin(cyclepercent);
               local y2 = math.cos(cyclepercent);
               -- helix r1
               do
                  local bullet = bullet_new(BULLET_SOURCE_ENEMY);
                  bullet_set_position(bullet, 
                  ex + bullet_direction_perp[1]*y1*helix_height, 
                  ey + bullet_direction_perp[2]*y1*helix_height);
                  bullet_set_visual(bullet, bullet_visual);
                  bullet_set_lifetime(bullet, 15);
                  bullet_set_scale(bullet, 3, 3);
                  bullet_start_trail(bullet, tcount);
                  bullet_set_trail_modulation(bullet, 0.8,0.8,0.8,0.3);
                  bullet_set_visual_scale(bullet, 0.3, 0.3);
                  bullet_set_velocity(bullet, bullet_velocity[1], bullet_velocity[2]);
               end
               -- -- helix r2
               do
                  local bullet = bullet_new(BULLET_SOURCE_ENEMY);
                  bullet_set_position(bullet, ex + bullet_direction_perp[1]*y2*helix_height , ey + bullet_direction_perp[2]*y2*helix_height);
                  bullet_set_visual(bullet, bullet_visual1);
                  bullet_set_lifetime(bullet, 15);
                  bullet_set_scale(bullet, 3, 3);
                  bullet_start_trail(bullet, tcount);
                  bullet_set_trail_modulation(bullet, 0.8,0.8,0.8,0.3);
                  bullet_set_visual_scale(bullet, 0.3, 0.3);
                  bullet_set_velocity(bullet, bullet_velocity[1], bullet_velocity[2]);
               end
               play_sound(random_attack_sound());
               t_wait(0.005);
            end
            t_wait(burst_delay);
         end

         t_wait(0.75);
         enemy_set_velocity(e, exit_direction[1] * exit_velocity, exit_direction[2] * exit_velocity);
         enemy_set_acceleration(e, exit_direction[1] * exit_acceleration, exit_direction[2] * exit_acceleration);
      end
   );
   return e;
end


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
      t_wait(12);
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
      t_wait(2);
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
         15,
         v2(play_area_width() + 10, play_area_height()/2),
         v2(play_area_width() - 50, play_area_height()/2 +10),
         0.85,

         0.65,
         3,

         25,
         125,

         5, 120,

         v2(1, 0),
         100,
         30,

         1,
         PROJECTILE_SPRITE_NEGATIVE,1
      );
      enemy_set_visual(e4, ENTITY_SPRITE_SKULL_B);
      t_wait(5);
      local e5 = Make_Enemy_Burst360_1_1_2_EX(
         15,
         v2(-10, play_area_height()/2),
         v2(50, play_area_height()/2 - 50),
         0.85,

         0.65,
         2,

         35,
         90,

         5, 125,

         v2(1, 0),
         100,
         30,

         1,
         PROJECTILE_SPRITE_RED
      );
      enemy_set_visual(e5, ENTITY_SPRITE_SKULL_B);

      local e6 = Make_Enemy_Burst360_1_1_2_EX(
         15,
         v2(play_area_width() + 10, play_area_height()/2),
         v2(play_area_width() - 50, play_area_height()/2 - 50),
         0.85,

         0.5,
         2,

         25,
         90,

         5, 120,

         v2(1, 0),
         100,
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
            t_wait(1);
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2 - 150,
               play_area_height()/2,
               ENTITY_SPRITE_SKULL_B,
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
               ENTITY_SPRITE_SKULL_B,
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

   for i=1,8 do
      do
         local e0 = Make_Enemy_SideMoverWave1_1_1(10 + i * 35, -100, 0, 100, 1.5, 1, 0, 12, PROJECTILE_SPRITE_RED_ELECTRIC, PROJECTILE_SPRITE_RED_DISK);
         enemy_set_visual(e0, ENTITY_SPRITE_SKULL_B);
         if i % 2 == 0 then
            t_wait(0.5);
         else
            t_wait(1);
         end
      end
   end
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

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg2_1.png", 0.250, 0, 230);
   play_area_notify_current_border_status();
   t_wait(3);
   wave1();
   t_wait(1);
   wave2();
   t_wait(12);
   convert_all_bullets_to_score();
   wait_no_danger();
   t_complete_stage();
end
