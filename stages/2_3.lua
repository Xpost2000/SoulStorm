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

function MakeBoneWorm_2_3_Simple(
    x, y, dir, speed,

    head_hp, baring_hp,

    slength,

    projectile_visual, visual, blowup_on_death,

    segment_tick,
    head_tick
)
    dir = v2_normalized(dir);
    local tail_visual = visual;
    if visual == ENTITY_SPRITE_SKULL_A then
        tail_visual = ENTITY_SPRITE_BONE_WORM_BARING_BONE;
    elseif visual == ENTITY_SPRITE_SKULL_B then
        tail_visual = ENTITY_SPRITE_BONE_WORM_BARING_GOLD;
    elseif visual == ENTITY_SPRITE_SKULL_A1 then
        tail_visual = ENTITY_SPRITE_BONE_WORM_BARING_CAUSTIC;
    elseif visual == ENTITY_SPRITE_SKULL_B1 then
        tail_visual = ENTITY_SPRITE_BONE_WORM_BARING_DEVIL;
    elseif visual == ENTITY_SPRITE_SKULL_HEXBINDER then
        tail_visual = ENTITY_SPRITE_BONE_WORM_BARING_HEXBINDER;
    end;

    print(tail_visual, "tail visual");

    local tails = {};
    for i=1,slength do
        local tail_entity = enemy_new();
        -- build in opposite direction of spawn
        local tail_position = v2(
            -- index order is calculated like this so slicing off the tail is easier
            x + -dir[1] * 16 * ((slength+1)-i),
            y + -dir[2] * 16 * ((slength+1)-i)
        );
    
        enemy_set_hp(tail_entity, baring_hp);
        enemy_set_position(tail_entity, tail_position[1], tail_position[2]);
        enemy_set_visual(tail_entity, tail_visual);
        enemy_set_scale(tail_entity, 8, 8);
        if tail_tick ~= nil then
            enemy_task_lambda(tail_entity,
            function (e)
                while enemy_valid(e) do
                    tail_tick();
                    t_yield();
                end
            end);
        end
        tails[i] = tail_entity;
    end
    
    local head_entity = enemy_new();
    enemy_set_hp(head_entity, head_hp);
    enemy_set_position(head_entity, x, y);
    enemy_set_visual(head_entity, visual);
    enemy_set_scale(head_entity, 10, 10);
    if head_tick ~= nil then
         enemy_task_lambda(state.head, 
         function (e)
             while enemy_valid(e) do
                 head_tick(e);
                 t_yield();
             end
         end);
    end

    local selfstate = {
        head = head_entity,
        tails = tails
    }; -- thank god for closures.
    
    -- main lambda update on the head
    --local speed=72;
    local slink_speed_factor=4;
    local slink_magnitude=16;
    async_task_lambda(
        function(state)
            while (enemy_valid(state.head)) do
                -- snake tail moves in a sine wave
                -- but need to define in terms of derivatives
                for i=1,slength do
                    local v = state.tails[i];
                    if v ~= nil then
                        if (enemy_valid(v)) then
                            local lftime = enemy_time_since_spawn(v);
                            -- contribution is the norm of the movement, amplitude is at most the sprite size (32), otherwise there's disconnect
                            -- also chain rule application for (d/dx) sin(kx) = cos(kx), k = constant
                            enemy_set_velocity(v, 
                                slink_magnitude*(math.cos(slink_speed_factor*(lftime + i)) * -dir[2]) 
                                + dir[1]*speed, 
                                slink_magnitude*(math.cos(slink_speed_factor*(lftime + i)) * dir[1])  
                                + (dir[2]*speed));
                        else
                            -- blow up all segments "behind" me
                            if blowup_on_death then
                                async_task_lambda(function()
                                    for j=1,i-1 do
                                        local v = state.tails[j];
                                        if (enemy_valid(v)) then
                                            enemy_kill(v, 1);
                                            random_explosion_sound();
                                        end
                                        t_wait(0.035); -- this will look satisfying!!!
                                    end
                                end);
                            else
                                -- disconnect behind segments
                                for j=1,i-1 do
                                    local v = state.tails[j];
                                    if v ~= nil then
                                        print("rip off ", j, " for death on ", i);
                                        state.tails[j] = nil;
                                    end
                                end
                            end
                        end
                    end
                end
                enemy_set_velocity(state.head, dir[1]*speed, dir[2]*speed);
                t_yield();
            end

            print("MY HEAD DIED");
            if blowup_on_death then
                for i, v in ipairs(state.tails) do
                    if (enemy_valid(v)) then
                        enemy_kill(v, 1);
                        random_explosion_sound();
                    end
                    t_wait(0.035); -- this will look satisfying!!!
                end
            end
            return;
        end,
        selfstate
    );

    return head_entity;
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

   if true then
     MakeBoneWorm_2_3_Simple(
         play_area_width()/2, play_area_height()/2, v2(1, 0),16,

         64, 16,

         8,
         PROJECTILE_SPRITE_PURPLE_ELECTRIC, ENTITY_SPRITE_SKULL_A, false
     )
   end

   if false then
       t_wait(1.5);
       wave1();
       t_wait(0.84);
       wave1();
       t_wait(0.45)
       -- Game_Spawn_Stage3_1_SubBoss();
       Game_Spawn_Stage2_2_SubBoss();
   -- Game_Spawn_Stage2_2_SubBoss();
   end
end
