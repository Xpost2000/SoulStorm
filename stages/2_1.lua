engine_dofile("stages/common.lua")

function wave1()
   -- Main encounter is a spinster wheel blade with various popcorn enemies and some shooters
   -- this should buy me some 30 seconds. Main focus is to try and survive the spin projectiles

   async_task_lambda( -- NOTE: async timeline
      function()
         DramaticExplosion_SpawnSpinnerObstacle1_2_1(
            play_area_width()/2,
            play_area_height()/2,
            ENTITY_SPRITE_SKULL_A,
            5,
            30,
            PROJECTILE_SPRITE_GREEN,
            v2(0, 1),
            15
         );
      end
   )
   async_task_lambda( -- NOTE: async timeline, throw some popcorn 
      function()
         t_wait(3.0);
         -- NOTE: due to some "historic" api mistake these guys don't spawn all at once.
         -- and actually influence the timer lol.
         do
            async_task_lambda(
               function()
                  Make_BrainDead_Enemy_Popcorn1(
                     8,
                     v2(80, -15),
                     0.0756,
                     10,
                     3,
                     45,
                     15,
                     20,
                     2,
                     -1
                  );
               end
            )
            async_task_lambda(
               function()
                  Make_BrainDead_Enemy_Popcorn1(
                     8,
                     v2(play_area_width()-80, -15),
                     0.0756,
                     10,
                     3,
                     45,
                     15,
                     20,
                     2,
                     -1
                  );
               end
            )
         end
         t_wait(4.0);
         do
            async_task_lambda(
               function()
                  Make_BrainDead_Enemy_Popcorn1(
                     8,
                     v2(80, -15),
                     0.0756,
                     10,
                     3,
                     120,
                     15,
                     20,
                     2,
                     -1
                  );
               end
            )
            async_task_lambda(
               function()
                  Make_BrainDead_Enemy_Popcorn1(
                     8,
                     v2(play_area_width()-80, -15),
                     0.0756,
                     10,
                     3,
                     120,
                     15,
                     20,
                     2,
                     -1
                  );
               end
            )
         end
         t_wait(4.0);
         do
            Make_BrainDead_Enemy_Popcorn1(
               48,
               v2(play_area_width() + 20, play_area_height()/2),
               0.0956,
               5,
               -120,
               15,
               20,
               20,
               -1,
               -1
            );
         end
      end
   )

   -- NOTE: absolute time 13 seconds ish
   t_wait(14.0);
   -- Spawn a few bursters to force dodging harder
   do
      local e0 = Make_Enemy_Burst360_1_1_2(
         30,
         v2(40, -20),
         v2(40, 40),
         0.5,
         0.15,
         8,

         45,
         8,

         40,
         80,
         dir_to_player(v2(40, 40)),
         70,
         10,
         
         PROJECTILE_SPRITE_RED
      )
      enemy_set_visual(e0, ENTITY_SPRITE_SKULL_A);

      local e1 = Make_Enemy_Burst360_1_1_2(
         30,
         v2(play_area_width() - 40, -20),
         v2(play_area_width() - 40, 40),
         0.5,
         0.15,
         8,

         45,
         8,

         40,
         80,
         dir_to_player(v2(40, 40)),
         70,
         10,

         PROJECTILE_SPRITE_RED
      )
      enemy_set_visual(e1, ENTITY_SPRITE_SKULL_A);
   end

   -- Spawn a small semi circle of 4 exploder enemies
   -- it should be possible to dodge them once they're killed. I hope. If not, I'll just make the explosion
   -- radius pretty small.
end

-- More of a good thing is more good things!
-- More weavers and bringing back the spinster from wave 1.
function wave2()
   -- Spawn the main obstacles
   do
      async_task_lambda( -- NOTE: async timeline
         function()
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2,
               play_area_height()/2 - 150,
               ENTITY_SPRITE_SKULL_B,
               10,
               30,
               PROJECTILE_SPRITE_GREEN_DISK,
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
               25,
               PROJECTILE_SPRITE_RED_DISK,
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
               25,
               PROJECTILE_SPRITE_BLUE_DISK,
               v2(-1, 1)
            );
         end
      )
   end
   t_wait(3.5);

   -- some extra enemies...
   do
      local e0 = Make_Enemy_Burst360_1_1_2(
         15,
         v2(-10, play_area_height()/2),
         v2(50, play_area_height()/2 - 50),
         0.85,

         0.5,
         2,
         25,
         45,

         15, 45,

         v2(1, 0),
         100,
         30,

         PROJECTILE_SPRITE_RED_STROBING
      );
      enemy_set_visual(e0, ENTITY_SPRITE_SKULL_A);

      local e1 = Make_Enemy_Burst360_1_1_2(
         15,
         v2(play_area_width() + 10, play_area_height()/2),
         v2(play_area_width()/2 + 50, play_area_height()/2 - 50),
         0.85,

         0.5,
         2,
         25,
         45,

         15, 45,

         v2(1, 0),
         100,
         30,

         PROJECTILE_SPRITE_RED_STROBING
      );
      enemy_set_visual(e1, ENTITY_SPRITE_SKULL_A);
   end

   -- Laser torment
   t_wait(7);
   LaserChaser_Horizontal_1_2(2, 2.5);
   t_wait(4);
   LaserChaser_Vertical_1_2(2, 2.5);
   t_wait(3);
end

function wave3()
   -- figure out what time this is
   -- Add a few of the level 1 Home attackers
   local e0 = Make_Enemy_SideMoverWave1_1_1(100, -100, 0, 200, 1.5, 1, 0, 8, PROJECTILE_SPRITE_RED_ELECTRIC, PROJECTILE_SPRITE_RED_DISK);
   enemy_set_visual(e0, ENTITY_SPRITE_SKULL_B);
   local e1 = Make_Enemy_SideMoverWave1_1_1(play_area_width() - 100, -100, 0, 200, 1.5, 1, 0, 4, PROJECTILE_SPRITE_GREEN_ELECTRIC, PROJECTILE_SPRITE_GREEN_DISK);
   enemy_set_visual(e1, ENTITY_SPRITE_SKULL_B);
   local e2 = Make_Enemy_SideMoverWave1_1_1(150, -100, 0, 200, 1.0, 1, 0, 8, PROJECTILE_SPRITE_RED_ELECTRIC, PROJECTILE_SPRITE_RED_DISK);
   enemy_set_visual(e2, ENTITY_SPRITE_SKULL_B);
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
                           0, 40, 1.0, 0, 4, PROJECTILE_SPRITE_BLUE_STROBING, 0,
                           15, 20, 15
   );
   enemy_set_visual(e4, ENTITY_SPRITE_BAT_B1);
   t_wait(1.5);
   local e5 = Make_Enemy_Spinster_1_1(-20,
                           play_area_height()/2,
                           40, 0, 1.0, 0, 3, PROJECTILE_SPRITE_GREEN_STROBING, -90,
                           15, 20, 25
   );
   enemy_set_visual(e5, ENTITY_SPRITE_BAT_B1);
   local e6 = Make_Enemy_Spinster_1_1(play_area_width()+20,
                           play_area_height()/2,
                           -20, 0, 1.0, 0, 3, PROJECTILE_SPRITE_GREEN_STROBING, 90,
                           15, 20, 25
   );
   enemy_set_visual(e6, ENTITY_SPRITE_BAT_B1);
   -- Add a nearly impossible scenario (dodge by staying in the center)
   t_wait(4);
   Make_BrainDead_Enemy_Popcorn1(
      20,
      v2(130, -10),
      0.026,
      7,
      0,
      100,
      25,
      2,
      4,
      -1);
   Make_BrainDead_Enemy_Popcorn1(
      20,
      v2(play_area_width() - 130, -10),
      0.026,
      7,
      0,
      100,
      25,
      2,
      4,
      -1);
   t_wait(5);

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
               PROJECTILE_SPRITE_GREEN_DISK,
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
               25,
               PROJECTILE_SPRITE_GREEN_DISK,
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
               25,
               PROJECTILE_SPRITE_GREEN_DISK,
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

         PROJECTILE_SPRITE_RED_STROBING
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

         PROJECTILE_SPRITE_RED_STROBING
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

   -- This might be pretty hard.
   t_wait(3.5);
   explosion_hazard_new(player_position_x(), player_position_y(), 50, 1, 1);
   t_wait(6.5);
   explosion_hazard_new(player_position_x(), player_position_y(), 80, 1, 1);
   t_wait(9.5);
   explosion_hazard_new(player_position_x(), player_position_y(), 120, 1, 1);
   enable_grazing();
   enable_bullet_to_points();

   -- Add an enemy flood wave (should be dog sprites [TODO: modify dog hero sprite])
   -- and use them as the new enemies.
end

function wave4()
   do
      DramaticExplosion_SpawnShotgunSpread(
         play_area_width() * 0.25, 20, ENTITY_SPRITE_SKULL_B,
         50, 6, 10, PROJECTILE_SPRITE_GREEN_STROBING, 1.5, 0.55, v2(0, 1), 90,
         10
      );
      DramaticExplosion_SpawnShotgunSpread(
         play_area_width() * 0.65, 20, ENTITY_SPRITE_SKULL_B,
         50, 8, 10, PROJECTILE_SPRITE_GREEN_STROBING, 1.5, 0.55, v2(0, 1), 90,
         10
      );

      DramaticExplosion_SpawnShotgunSpread(
         play_area_width() * 0.35, 10, ENTITY_SPRITE_SKULL_B,
         50, 8, 10, PROJECTILE_SPRITE_RED_STROBING, 1.5, 0.55, v2(0.2, 1), 55,
         15
      );
      DramaticExplosion_SpawnShotgunSpread(
         play_area_width() * 0.45, 10, ENTITY_SPRITE_SKULL_B,
         50, 6, 10, PROJECTILE_SPRITE_RED_STROBING, 1.5, 0.55, v2(-0.2, 1), 55,
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
         25,
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
end

function stage_task()
   play_area_set_edge_behavior(PLAY_AREA_EDGE_TOP, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_BOTTOM, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_LEFT, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_RIGHT, PLAY_AREA_EDGE_BLOCKING);
   -- setup stage background
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage2bkg_1.png", 0.50, 0, 0); -- TODO come up with new design

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star0.png", 0.357, 225, 125);
   -- Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star1.png", 0.147, -150, 245);
   -- Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.287, 305, 300);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.207, -100, 225);

   -- Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.127, 100, 15);
   -- Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.257, -350, 255);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage2bkg2_1.png", 0.150, 0, 125);
   play_area_notify_current_border_status();
   -- end stage setup background

   t_wait(1.5);
   wave1();
   t_wait(22);
   wave2();
   t_wait(3.5);
   wave3();
   t_wait(17);
   convert_all_enemies_to_score();
   convert_all_bullets_to_score();
   t_wait(3);
   wave4();
   
   wait_no_danger();
   t_complete_stage();
end
