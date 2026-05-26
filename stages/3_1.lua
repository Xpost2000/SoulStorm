engine_dofile("stages/common.lua")
engine_dofile("stages/subboss22.lua")

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
function wave1()
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
   t_wait(1.5);
   sub00();
   t_wait(2.5);
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
    t_wait(3.5);
    sub00();
    t_wait(3.5);

    BOSS_HP=1800; -- for lesser bone wings.
    local bonewings1 = Game_Spawn_Stage2_2_SubBoss();

    -- wait until mini boss is dead.
    t_wait(1.25);

    local bosstspawn = enemy_time_since_spawn(bonewings1);
    while enemy_valid(bonewings1) and ((enemy_time_since_spawn(bonewings1)-bosstspawn) < 13) do
        t_yield();
    end
    EARLY_BYE=true;
    t_wait(4.5);

    MainBoss1_RainCloud_Attack2(2026, 7.7)
    async_task_lambda( -- NOTE: async timeline
        function()
        DramaticExplosion_SpawnSpinnerObstacle1_2_1(
            play_area_width()/2,
            play_area_height()/2 - 150,
            ENTITY_SPRITE_SKULL_B1,
            23,
            15,
            PROJECTILE_SPRITE_WRM,
            v2(0, 1)
        );
        end
    )
    t_wait(8.35);

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
    t_wait(2);
    sub00();
end

function stage_task()
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage3bkg1_0.png", 0.28, 0, 0);

   play_area_set_edge_behavior(PLAY_AREA_EDGE_TOP, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_BOTTOM, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_LEFT, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_RIGHT, PLAY_AREA_EDGE_BLOCKING);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star0.png", 0.487, 225, 85);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star1.png", 0.287, -120, 175);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.187, 355, 15);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.117, -150, 255);

   t_wait(2.25);

   wave1();

   wait_no_danger();
   t_complete_stage();
end
