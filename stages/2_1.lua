-- TODO:
-- Finish Wave1 30 seconds.

engine_dofile("stages/common.lua")

-- Enemy that spawns in a place, does a spin dodge trap
-- linear.
-- Will exit after duration is over.
-- These are very specific behaviors tbh.
function Make_Enemy_SpinTrip_2_1_1(hp,
                                   position,
                                   spoke_arc,
                                   spoke_per_frame_angular_velocity,
                                   exit_direction,
                                   exit_speed,
                                   exit_acceleration,
                                   duration,
                                   bullet_visual
                                  )
   local e = enemy_new();
   enemy_set_hp(e, hp);
   enemy_set_position(e, position[1], position[2]);

   exit_direction = v2_normalized(exit_direction);

   enemy_task_lambda(
      e,
      function(e)
         local start_time = enemy_time_since_spawn(e);
         local arc_displacement = 0;

         while (enemy_time_since_spawn(e) - start_time < duration) do
            local bspeed    = 50;
            local eposition = enemy_final_position(e);

            for angle=1,360,spoke_arc do
               local bullet = bullet_new(BULLET_SOURCE_ENEMY);
               bullet_set_position(bullet, eposition[1], eposition[2]);
               bullet_set_visual(bullet, bullet_visual);
               bullet_set_lifetime(bullet, 15);
               bullet_set_scale(bullet, 3, 3);
               bullet_set_visual_scale(bullet, 0.3, 0.3);

               local bdir = v2_direction_from_degree(angle + arc_displacement);
               bullet_set_velocity(bullet, bdir[1] * bspeed, bdir[2] * bspeed);
            end

            arc_displacement = arc_displacement + spoke_per_frame_angular_velocity;
            t_wait(0.25);
         end

         enemy_move_linear(e, exit_direction, exit_speed);
         enemy_set_acceleration(e, exit_direction[1] * exit_acceleration, exit_direction[2] * exit_acceleration);
      end
   )
end

function wave1()
   -- Main encounter is a spinster wheel blade with various popcorn enemies and some shooters
   -- this should buy me some 30 seconds. Main focus is to try and survive the spin projectiles

   async_task_lambda( -- NOTE: async timeline
      function()
         do
            local enemy_position = v2(play_area_width()/2, play_area_height()/2);
            explosion_hazard_new(enemy_position[1], enemy_position[2], 15, 0.05, 0.15);
            t_wait(1.25);
            Make_Enemy_SpinTrip_2_1_1(9999,
                                      enemy_position,
                                      60,
                                      5,
                                      v2(0, 1),
                                      50,
                                      30,
                                      30.0,
                                      PROJECTILE_SPRITE_GREEN);
         end
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

   t_wait(10.0);
   -- Spawn a small semi circle of 4 exploder enemies
   -- it should be possible to dodge them once they're killed. I hope. If not, I'll just make the explosion
   -- radius pretty small.
end

function stage_task()
   t_wait(1.5);

   wave1();

   t_wait(10);
   wait_no_danger();
   t_complete_stage();
end
