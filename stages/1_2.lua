dofile("stages/common.lua")

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

function wave1_sub1()
   -- smoothly open with a few sprayers (with some slow bullets)
   -- wait a bit
   -- have some popcorns (through the center line)
   -- then a few more sprayers.
   -- popcorn
   do
      Make_Enemy_Burst360_1_1_2(
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

      Make_Enemy_Burst360_1_1_2(
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
         Make_Enemy_Burst360_1_1_2(
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
         
         Make_Enemy_Burst360_1_1_2(
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

         Make_Enemy_Burst360_1_1_2(
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
         
         Make_Enemy_Burst360_1_1_2(
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
end

function wave_1()
   -- do
   --    local e = enemy_new();
   --    enemy_set_position(e, 100, 100);
   --    enemy_set_velocity(e, 10, 0);
   --    local shield_r = 35;
   --    for angle=1,360,40 do
   --       local current_arc_direction = v2_direction_from_degree(angle % 360);
   --       local position = enemy_position(e);
   --       position[1] = position[1] + current_arc_direction[1] * shield_r;
   --       position[2] = position[2] + current_arc_direction[2] * shield_r;
   --       local b = bullet_make(
   --          BULLET_SOURCE_ENEMY,
   --          position,
   --          PROJECTILE_SPRITE_HOT_PINK_ELECTRIC,
   --          v2(0.5, 0.5),
   --          v2(5, 5)
   --       );

   --       bullet_task_lambda(
   --          b,
   --          function (b, master)
   --             local starting_angle = dir_to_angle(current_arc_direction);
   --             local t = 0;
   --             bullet_reset_movement(b);
   --             while enemy_valid(e) do
   --                bullet_set_velocity(b,
   --                                    -math.sin(math.rad(starting_angle+t)) * shield_r + enemy_velocity_x(master),
   --                                    math.cos(math.rad(starting_angle+t)) * shield_r  + enemy_velocity_y(master));
   --                t = t + 1;
   --                t_yield();
   --             end
   --          end,
   --          e
   --       );
   --    end
   -- end
   wave1_sub1();
end

function stage_task()
   t_wait(2);
   wave_1();
   t_wait(5);
   wait_no_danger();
   t_complete_stage();
end
