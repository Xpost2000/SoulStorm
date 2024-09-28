-- TODO:
-- When I am ready to return, so I shall!

-- TODO:
-- Finish Wave1 30 seconds.
-- NOTE (00:00::36) is empty spot for next round!

engine_dofile("stages/common.lua")

-- Not really meant to be killable, but you can farm points off of them!
function DramaticExplosion_SpawnSpinnerObstacle1_2_1(x,
                                                 y,
                                                 enemy_visual,
                                                 spoke_per_frame_angular_velocity,
                                                 duration,
                                                 bullet_visual,
                                                 exit_direction)
   local enemy_position = v2(x, y);
   explosion_hazard_new(enemy_position[1], enemy_position[2], 15, 0.05, 0.15);
   t_wait(1.25);
   local e = Make_Enemy_SpinTrip_2_1_1(9999,
                                       enemy_position,
                                       60,
                                       spoke_per_frame_angular_velocity,
                                       exit_direction,
                                       50,
                                       30,
                                       duration,
                                       bullet_visual);

   enemy_set_visual(e, enemy_visual);
   enemy_set_visual_scale(e, 1.3, 1.3);
end

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
            v2(0, 1)
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
      Make_Enemy_Burst360_1_1_2(
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

      Make_Enemy_Burst360_1_1_2(
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
      Make_Enemy_Burst360_1_1_2(
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

      Make_Enemy_Burst360_1_1_2(
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
   end

   -- Laser torment
   t_wait(7);
   LaserChaser_Horizontal_1_2(4, 2.5);
   t_wait(4);
   LaserChaser_Vertical_1_2(4, 2.5);
   t_wait(3);
end

function wave3()
   -- -- figure out what time this is
   -- -- Add a few of the level 1 Home attackers
   -- Make_Enemy_SideMoverWave1_1_1(100, -100, 0, 200, 1.5, 1, 0, 8, PROJECTILE_SPRITE_RED_ELECTRIC, PROJECTILE_SPRITE_RED_DISK);
   -- Make_Enemy_SideMoverWave1_1_1(play_area_width() - 100, -100, 0, 200, 1.5, 1, 0, 4, PROJECTILE_SPRITE_GREEN_ELECTRIC, PROJECTILE_SPRITE_GREEN_DISK);
   -- Make_Enemy_SideMoverWave1_1_1(150, -100, 0, 200, 1.0, 1, 0, 8, PROJECTILE_SPRITE_RED_ELECTRIC, PROJECTILE_SPRITE_RED_DISK);
   -- Make_Enemy_SideMoverWave1_1_1(play_area_width() - 150, -100, 0, 200, 1.0, 1, 0, 4, PROJECTILE_SPRITE_GREEN_ELECTRIC, PROJECTILE_SPRITE_GREEN_DISK);
   -- -- some popcorns,
   -- t_wait(2);

   -- -- TODO replace with dog flood
   -- Stage1_Batflood();
   -- t_wait(4);
   -- -- Add a spinster (stage 1 style)
   -- Make_Enemy_Spinster_1_1(play_area_width()/2,
   --                         -20,
   --                         0, 40, 1.0, 0, 4, PROJECTILE_SPRITE_BLUE_STROBING, 0,
   --                         15, 50, 55
   -- );
   -- Make_Enemy_Spinster_1_1(play_area_width()/2,
   --                         play_area_height() + 20,
   --                         0, -40, 1.0, 0, 4, PROJECTILE_SPRITE_BLUE_STROBING, 180,
   --                         15, 50, 55
   -- );
   -- t_wait(1.5);
   -- Make_Enemy_Spinster_1_1(-20,
   --                         play_area_height()/2,
   --                         40, 0, 1.0, 0, 3, PROJECTILE_SPRITE_GREEN_STROBING, -90,
   --                         15, 80, 85
   -- );
   -- Make_Enemy_Spinster_1_1(play_area_width()+20,
   --                         play_area_height()/2,
   --                         -20, 0, 1.0, 0, 3, PROJECTILE_SPRITE_GREEN_STROBING, 90,
   --                         15, 80, 85
   -- );
   -- -- Add a nearly impossible scenario (dodge by staying in the center)
   -- t_wait(4);
   -- Make_BrainDead_Enemy_Popcorn1(
   --    20,
   --    v2(130, -10),
   --    0.026,
   --    7,
   --    0,
   --    100,
   --    25,
   --    2,
   --    4,
   --    -1);
   -- Make_BrainDead_Enemy_Popcorn1(
   --    20,
   --    v2(play_area_width() - 130, -10),
   --    0.026,
   --    7,
   --    0,
   --    100,
   --    25,
   --    2,
   --    4,
   --    -1);
   -- t_wait(5);

   -- harder version of wave2
   do
      async_task_lambda( -- NOTE: async timeline
         function()
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2,
               play_area_height()/2 - 150,
               ENTITY_SPRITE_SKULL_B,
               10,
               30,
               PROJECTILE_SPRITE_NEGATIVE_DISK,
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
               PROJECTILE_SPRITE_NEGATIVE_DISK,
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
               PROJECTILE_SPRITE_NEGATIVE_DISK,
               v2(-1, 1)
            );
         end
      )

      async_task_lambda( -- NOTE: async timeline
         function()
            DramaticExplosion_SpawnSpinnerObstacle1_2_1(
               play_area_width()/2,
               play_area_height() -40,
               ENTITY_SPRITE_SKULL_B,
               10,
               30,
               PROJECTILE_SPRITE_NEGATIVE_DISK,
               v2(0, -1)
            );
         end
      )
   end

   -- Add an enemy flood wave (should be dog sprites [TODO: modify dog hero sprite])
   -- and use them as the new enemies.
end

function stage_task()
   -- t_wait(1.5);

   -- wave1();
   -- t_wait(22);
   -- wave2();
   -- t_wait(3.5);
   wave3();
   t_wait(5);
   
   wait_no_danger();
   t_complete_stage();
end
