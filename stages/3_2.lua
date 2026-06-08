engine_dofile("stages/common.lua")
engine_dofile("stages/subboss23.lua")

function _Stage_Boss_Spoke_Of_The_WheelAttack(e)
    disable_bullet_to_points();
    local running = true;
    local radius_buildup=1;
    start_black_fade(0.05);
    async_task_lambda(function(e)
        while running and enemy_valid(e) do
            local position = v2(enemy_position_x(e), enemy_position_y(e));
            -- particles are "animated"
            local particle_emitter_ptr = enemy_get_particle_emitter(e, 1);
            particle_emitter_set_active(particle_emitter_ptr, true);
            particle_emitter_set_lifetime(particle_emitter_ptr, 0.15);
            particle_emitter_set_scale(particle_emitter_ptr, 1);
            particle_emitter_set_max_emissions(particle_emitter_ptr, -1);
            particle_emitter_set_emit_per_emission(particle_emitter_ptr, 128);
            particle_emitter_set_emission_max_timer(particle_emitter_ptr, 0.01);
            particle_emitter_set_acceleration(particle_emitter_ptr, 0, 98);
            particle_emitter_set_velocity_x_variance(particle_emitter_ptr, -64, 64);
            particle_emitter_set_velocity_y_variance(particle_emitter_ptr, -64, 64);
            particle_emitter_set_scale_variance(particle_emitter_ptr, 0.02, 0.1);
            particle_emitter_set_emit_shape_circle(particle_emitter_ptr, position[1], position[2], radius_buildup, false);
            particle_emitter_set_sprite_projectile(particle_emitter_ptr, PROJECTILE_SPRITE_SPARKLING_STAR);
            t_yield();
        end
    end, e);

    local spokes = 16;
    local spoke_angle = 360 / spokes;
    local preturn = 30;
    local laser_speed= 7200;

    function grow_spoke(turn)
       -- fake engine lasers, cause I don't have real ones
       -- on the brightside, it means we can attack the laser spiral
       local position = v2(enemy_position_x(e), enemy_position_y(e));
       for j=4,30 do
          for i=0,spokes do
             local ang = spoke_angle * i + turn;
             do
                local bullet = bullet_new(BULLET_SOURCE_ENEMY);
                local fire_dir = v2_direction_from_degree(ang);

                bullet_set_position(bullet, position[1]+fire_dir[1]*16*j, position[2]+fire_dir[2]*16*j);
                bullet_set_visual(bullet, PROJECTILE_SPRITE_LASER_PURPLE);
                bullet_set_lifetime(bullet, 0.93);
                bullet_set_scale(bullet, 5, 5);
                bullet_set_visual_scale(bullet, 1, 1);

                bullet_set_velocity(bullet, fire_dir[1] * 0.001, fire_dir[2] * 0.001);
             end
          end
          t_yield();
       end
    end

    async_task_lambda(
       grow_spoke,
       preturn
    );
    t_wait(0.50);
   

    while enemy_valid(e) and running do
        local position = v2(enemy_position_x(e), enemy_position_y(e));
        local velocity = v2(enemy_velocity_x(e), enemy_velocity_y(e));

        function create_spoke(turn)
           -- fake engine lasers, cause I don't have real ones
           -- on the brightside, it means we can attack the laser spiral
           for j=4,10 do
              for i=0,spokes do
                 local ang = spoke_angle * i + turn;
                 do
                    local bullet = bullet_new(BULLET_SOURCE_ENEMY);
                    local fire_dir = v2_direction_from_degree(ang);

                    bullet_set_position(bullet, position[1]+fire_dir[1]*16*j, position[2]+fire_dir[2]*16*j);
                    bullet_set_visual(bullet, PROJECTILE_SPRITE_LASER_PURPLE);
                    bullet_set_lifetime(bullet, 0.75);
                    bullet_set_scale(bullet, 5, 5);
                    bullet_set_visual_scale(bullet, 1, 1);

                    bullet_set_velocity(bullet, fire_dir[1] * laser_speed, fire_dir[2] * laser_speed);
                 end
              end
           end
        end


        while radius_buildup < 48 do
            radius_buildup = radius_buildup + 1.5;
            if radius_buildup > 48 then
                radius_buildup = 48;
            end
            t_yield();
        end

        for adj=0,120 do
            preturn = preturn + 0.3;
            create_spoke(preturn);
            t_yield();
        end
        
        for adj=0,240 do
            preturn = preturn - 0.3;
            create_spoke(preturn);
            t_yield();
        end

        running = false;
        break;
    end
    end_black_fade(0.05);
end

function Game_Spawn_Stage_BossCameo()
   local e = enemy_new();
   local initial_boss_pos = v2(play_area_width()/2, 200);
   enemy_set_hp(e, 9999999);
   enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
   enemy_set_visual(e, ENTITY_SPRITE_BOSS3_FACE_FRONT);
   enemy_set_burst_gain_value(e, 0.55);
   enemy_show_boss_hp(e, "???");

   -- The boss takes "three threads" of logic.
   -- async_task_lambda(_Stage1_Boss_Logic, e);
   -- async_task_lambda(_Stage1_Boss_Maintain_Hexes_Logic, e);
   -- async_task_lambda(_Stage1_Boss_ImmunityToBurstLaser_Logic, e);

   return e;
end

function wave1()
   if true then
      -- want to use boneworm here. would be neat.
      t_wait(4);
      do
         DramaticExplosion_SpawnShotgunSpread(
            play_area_width() * 0.25, 20, ENTITY_SPRITE_SKULL_B,
            70, 6, 10, PROJECTILE_SPRITE_PURPLE_STROBING, 1.5, 0.55, v2(0, 1), 45,
            10
         );
         DramaticExplosion_SpawnShotgunSpread(
            play_area_width() * 0.65, 20, ENTITY_SPRITE_SKULL_B1,
            70, 8, 10, PROJECTILE_SPRITE_PURPLE_STROBING, 1.5, 0.55, v2(0, 1), 45,
            10
         );

         DramaticExplosion_SpawnShotgunSpread(
            play_area_width() * 0.35, 10, ENTITY_SPRITE_SKULL_B,
            70, 8, 10, PROJECTILE_SPRITE_PURPLE_STROBING, 1.5, 0.55, v2(0.2, 1), 55,
            15
         );
         DramaticExplosion_SpawnShotgunSpread(
            play_area_width() * 0.45, 10, ENTITY_SPRITE_SKULL_B1,
            70, 6, 10, PROJECTILE_SPRITE_PURPLE_STROBING, 1.5, 0.55, v2(-0.2, 1), 55,
            15
         );
      end
      t_wait(2.5);
      do
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
      end;
      t_wait(5);
      Stage1_Batflood();
      t_wait(1.5);
      Stage1_Batflood();
      t_wait(2);
      do
         async_task_lambda( -- NOTE: async timeline
            function()
               DramaticExplosion_SpawnSpinnerObstacle1_2_1(
                  play_area_width()/2,
                  play_area_height()/2 - 150,
                  ENTITY_SPRITE_HEXBINDER,
                  10,
                  20,
                  PROJECTILE_SPRITE_RED,
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
                  ENTITY_SPRITE_HEXBINDER,
                  15,
                  15,
                  PROJECTILE_SPRITE_RED,
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
                  ENTITY_SPRITE_SKULL_B1,
                  15,
                  15,
                  PROJECTILE_SPRITE_GREEN,
                  v2(-1, 1)
               );
            end
         )
      end
      t_wait(5);
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
      t_wait(7.5);
   end


   -- BOSS_CAMEO
   do
      local enemy_position = v2(play_area_width()/2, 200);
      explosion_hazard_new(enemy_position[1], enemy_position[2], 15, 0.05, 0.15);
      t_wait(1.5);
      local e = Game_Spawn_Stage_BossCameo();
      async_task_lambda(_Stage_Boss_Spoke_Of_The_WheelAttack, e);
      t_wait(7.5);
      enemy_kill(e, 1);
   end;

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
   t_wait(9.35);

   BOSS0_HP = 2300;
   BOSS1_HP = 1000;
   Game_Spawn_Stage2_3_SubBoss();
end

function stage_task()
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage3bkg2_0.png", 0.50, 0, 0);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star0.png", 0.117, 225, 125);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star1.png", 0.147, -150, 245);

   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.27, 100, 65);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.37, 500, 200);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage2bkg2_1.png", 0.180, 0, 125);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg2_1_2.png", 0.150, 0, 305);

   wave1();

   t_wait(2.5);
   wait_no_danger();
   t_complete_stage();
end
