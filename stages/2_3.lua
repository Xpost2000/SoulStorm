engine_dofile("stages/common.lua")
--engine_dofile("stages/subboss31.lua")
-- engine_dofile("stages/subboss22.lua")
engine_dofile("stages/subboss23.lua")

-- engine_dofile("stages/subboss22.lua")
engine_dofile("stages/subboss22.lua")

function wave1()
-- 1
    do
      local e5 = Make_Enemy_Burst360_1_1_2_EX(
         750,
         v2(-10, play_area_height()/2),
         v2(50, play_area_height()/2 - 50),
         0.85,

         0.7,
         2,

         35,
         90,

         5, 125,

         v2(1, 0),
         180,
         30,

         1,
         PROJECTILE_SPRITE_RED,4
      );
      enemy_set_visual(e5, ENTITY_SPRITE_SKULL_B);

      local e6 = Make_Enemy_Burst360_1_1_2_EX(
         750,
         v2(play_area_width() + 10, play_area_height()/2),
         v2(play_area_width() - 50, play_area_height()/2 - 50),
         0.85,

         0.7,
         2,

         25,
         90,

         5, 120,

         v2(1, 0),
         180,
         30,

         1,
         PROJECTILE_SPRITE_RED,4
      );
      enemy_set_visual(e6, ENTITY_SPRITE_SKULL_B);
    end
-- 2
    t_wait(1.0);
    do
      local e5 = Make_Enemy_Burst360_1_1_2_EX(
         750,
         v2(-10, play_area_height()/2),
         v2(90, play_area_height()/2 - 80),
         0.85,

         0.4,
         4,

         85,
         90,

         5, 125,

         v2(1, 0),
         180,
         30,

         1,
         PROJECTILE_SPRITE_WRM_ELECTRIC,4
      );
      enemy_set_visual(e5, ENTITY_SPRITE_SKULL_B1);

      local e6 = Make_Enemy_Burst360_1_1_2_EX(
         750,
         v2(play_area_width() + 10, play_area_height()/2),
         v2(play_area_width() - 90, play_area_height()/2 - 80),
         0.85,

         0.4,
         4,

         60,
         90,

         5, 120,

         v2(1, 0),
         180,
         30,

         1,
         PROJECTILE_SPRITE_WRM_ELECTRIC,4
      );
      enemy_set_visual(e6, ENTITY_SPRITE_SKULL_B1);
    end
-- 3
    t_wait(1.0);
    do
      local e5 = Make_Enemy_Burst360_1_1_2_EX(
         750,
         v2(-10, play_area_height()/2),
         v2(90, play_area_height()/2 - 130),
         0.85,

         0.4,
         6,

         85,
         50,

         5, 125,

         v2(1, 0),
         180,
         30,

         1,
         PROJECTILE_SPRITE_PURPLE_ELECTRIC,4
      );
      enemy_set_visual(e5, ENTITY_SPRITE_SKULL_A1);

      local e6 = Make_Enemy_Burst360_1_1_2_EX(
         750,
         v2(play_area_width() + 10, play_area_height()/2),
         v2(play_area_width() - 90, play_area_height()/2 - 130),
         0.85,

         0.4,
         6,

         60,
         50,

         5, 120,

         v2(1, 0),
         180,
         30,

         1,
         PROJECTILE_SPRITE_PURPLE_ELECTRIC,4
      );
      enemy_set_visual(e6, ENTITY_SPRITE_SKULL_A1);
    end
-- 4
    t_wait(2);
    async_task_lambda(
       function()
          for i=1, 30 do
             E0_1_1S(
                v2(0, -10),
                v2(150, 205),
                dir_to_player(v2(0, -10)), -- SHOT DIR
                0.23, 0.50, -- FIRE SPEED, FIRE DELAY
                0.50, -- TIME UNTIL ACCEL
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
                0.40, -- TIME UNTIL ACCEL
                v2(400, 100) -- ACCEL WHERE
             );
             t_wait(0.17)
          end
       end
    )
    t_wait(8.0);
    do
      DramaticExplosion_SpawnShotgunSpread(
         play_area_width() * 0.25, 20, ENTITY_SPRITE_SKULL_B,
         50, 6, 10, PROJECTILE_SPRITE_PURPLE_STROBING, 1.5, 0.55, v2(0, 1), 45,
         10
      );
      DramaticExplosion_SpawnShotgunSpread(
         play_area_width() * 0.65, 20, ENTITY_SPRITE_SKULL_B1,
         50, 8, 10, PROJECTILE_SPRITE_PURPLE_STROBING, 1.5, 0.55, v2(0, 1), 45,
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
   for i=1,4 do
      local e = Make_Enemy_Spinner_1_1_2(
         45, 
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
      enemy_set_visual(e, ENTITY_SPRITE_SKULL_B1);
   end
   for i=1,4 do
      local e = Make_Enemy_Spinner_1_1_2(
         45, 
         v2(play_area_width()+15 + i*15, 20 + i * 45),
         v2(-1, 0),
         80,
         2.0,
         0.0,
         45,
         5,
         5,
         PROJECTILE_SPRITE_WRM_DISK,
         4
         );
      enemy_set_visual(e, ENTITY_SPRITE_SKULL_B1);
   end
   t_wait(2);
end

function stage_task()
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_2.png", 0.15, 0, -25);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.27, 355, 15);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.147, -150, 205);
   -- Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.450, 35, 0);
   -- Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.800, 85, 200);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.650, 450, 200);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/boss_0_light_streak.png", 0.150, 0, 125);
   -- t_complete_stage();
   play_area_set_edge_behavior(PLAY_AREA_EDGE_TOP, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_BOTTOM, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_LEFT, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_RIGHT, PLAY_AREA_EDGE_BLOCKING);

   t_wait(1.5);
   wave1();
   t_wait(0.84);
   wave1();
   t_wait(0.45)
   -- Game_Spawn_Stage3_1_SubBoss();
   Game_Spawn_Stage2_2_SubBoss();
   -- Game_Spawn_Stage2_2_SubBoss();
end
