-- Another fixed pattern cyclic boss, without any random
-- stages (most bosses might just be designed like this from here on out,
-- which makes Stage 1 incredibly inconsistent/different in quality compared to
-- everything else which is supposed to be significantly better otherwise.)

BOSS0_HP=2200;
BOSS1_HP=900;
boss_state_0 = {
   last_good_position,
   me,
   phase
};
boss_state_1 = {
   last_good_position,
   me,
   phase
};

-- BOSS 1

function BossFight_Complete()
   if (not enemy_valid(e)) and (not enemy_valid(e1)) then
      return true;
   end

   return false;
end

function _Boss_Intro0(e)
   enemy_begin_invincibility(e, true, 2.5);
   enemy_set_acceleration(e, 0, 120);
   t_wait(1.0);
   enemy_set_acceleration(e, 0, -120);
   t_wait(1.0);
   enemy_set_acceleration(e, 0, 0);
   enemy_set_velocity(e, 0, 0);
end

function SubBoss23_UnravelAttack1(epos, displacement, v)
    for i=0, 10 do
        local bspeed = i*25 + 60;
        for ang=90, 180, 16 do
            local arcdir = v2_direction_from_degree(ang-i + displacement);
            local b = bullet_make(
            BULLET_SOURCE_ENEMY,
            epos,
            v,
            v2(0.5, 0.5),
            v2(3, 3)
            )

            bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
        end
        for ang=0, 90, 15 do
            local arcdir = v2_direction_from_degree(ang+i + displacement);
            local b = bullet_make(
            BULLET_SOURCE_ENEMY,
            epos,
            v,
            v2(0.5, 0.5),
            v2(3, 3)
            )

            bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
        end
        t_wait(0.125);
    end
end

function SubBoss23_Sprinkler1_AttackPattern(e)
   for burst=0,10 do
      async_task_lambda(
         function()
            local bspeed = 100;
            for angle=20,140,10 do
               if not enemy_valid(e) then
                  return;
               end
               local epos = enemy_final_position(e);

               local arcdir = v2_direction_from_degree(angle+burst);
               local b = bullet_make(
                  BULLET_SOURCE_ENEMY,
                  epos,
                  PROJECTILE_SPRITE_RED,
                  v2(0.5, 0.5),
                  v2(3, 3)
               )

               bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
               t_wait(0.018);
            end
         end
      )
      async_task_lambda(
         function()
            local bspeed = 200;
            for angle=50,170,6 do
               if not enemy_valid(e) then
                  return;
               end
               local epos = enemy_final_position(e);

               local arcdir = v2_direction_from_degree(angle-burst);
               local b = bullet_make(
                  BULLET_SOURCE_ENEMY,
                  epos,
                  PROJECTILE_SPRITE_PURPLE,
                  v2(0.5, 0.5),
                  v2(3, 3)
               )

               bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
               t_wait(0.038);
            end
            for angle=170,30,-8 do
               if not enemy_valid(e) then
                  return;
               end
               local epos = enemy_final_position(e);

               local arcdir = v2_direction_from_degree(angle+burst);
               local b = bullet_make(
                  BULLET_SOURCE_ENEMY,
                  epos,
                  PROJECTILE_SPRITE_PURPLE,
                  v2(0.5, 0.5),
                  v2(3, 3)
               )

               bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
               t_wait(0.038);
            end
         end
      )
      t_wait(0.5);
   end
end

function SubBoss23_Sprinkler2_AttackPattern(e)
   for burst=0,10 do
      async_task_lambda(
         function()
            local bspeed = 50;
            for angle=20,140,15 do
               if not enemy_valid(e) then
                  return;
               end
               local epos = enemy_final_position(e);

               local arcdir = v2_direction_from_degree(angle+burst);
               local b = bullet_make(
                  BULLET_SOURCE_ENEMY,
                  epos,
                  PROJECTILE_SPRITE_RED,
                  v2(0.5, 0.5),
                  v2(3, 3)
               )

               bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
               t_wait(0.065);
            end
         end
      )
      async_task_lambda(
         function()
            local bspeed = 90;
            for angle=50,170,12 do
               if not enemy_valid(e) then
                  return;
               end
               local epos = enemy_final_position(e);

               local arcdir = v2_direction_from_degree(angle-burst);
               local b = bullet_make(
                  BULLET_SOURCE_ENEMY,
                  epos,
                  PROJECTILE_SPRITE_PURPLE,
                  v2(0.25, 0.25),
                  v2(2.5, 2.5)
               )

               bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
               t_wait(0.090);
            end
            for angle=170,30,-12 do
               if not enemy_valid(e) then
                  return;
               end
               local epos = enemy_final_position(e);

               local arcdir = v2_direction_from_degree(angle+burst);
               local b = bullet_make(
                  BULLET_SOURCE_ENEMY,
                  epos,
                  PROJECTILE_SPRITE_PURPLE,
                  v2(0.25, 0.25),
                  v2(2.5, 2.5)
               )

               bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
               t_wait(0.100);
            end
         end
      )
      t_wait(0.6);
   end
end

function _Boss_TakeOver0(e)
   enemy_set_acceleration(e, 120, 0);
   t_wait(1.0);
   enemy_set_acceleration(e, -120, 0);
   t_wait(1.0);
   enemy_set_acceleration(e, 0, 0);
   enemy_set_velocity(e, 0, 0);
end

function _Boss_Logic0(e)
   _Boss_Intro0(e);

   local summoned_next = false;

   while enemy_valid(e) do
      local epos = enemy_final_position(e);
      if boss_state_0.phase == 0 then
         -- This is a fixed pattern where I will always do the
         -- same thing and then spawn the next boss where I'll used a cyclic
         -- pattern like in 2-2.

         --
         -- NOTE(jerry):
         --
         -- ... I should really be trying to get the enemy position
         -- each time, but the design I use makes bosses immobile,
         -- which is fine for now, since the game is meant to be simple
         -- and my design skills are immature (even up until release), but
         -- this shouldn't really slide.
         --

         if true then
            async_task_lambda(SubBoss23_UnravelAttack1, epos, 30, PROJECTILE_SPRITE_RED);
            if not enemy_valid(e) then return end;
            t_wait(0.40)
            async_task_lambda(SubBoss23_UnravelAttack1, epos, 50, PROJECTILE_SPRITE_CAUSTIC);
            if not enemy_valid(e) then return end;

            t_wait(1.8);

            async_task_lambda(SubBoss23_UnravelAttack1, epos, -40, PROJECTILE_SPRITE_GREEN);
            async_task_lambda(SubBoss23_UnravelAttack1, epos, 40, PROJECTILE_SPRITE_GREEN);
            if not enemy_valid(e) then return end;
            t_wait(1.20)
            async_task_lambda(SubBoss23_UnravelAttack1, epos, -70, PROJECTILE_SPRITE_BLUE);
            t_wait(0.5)
            async_task_lambda(SubBoss23_UnravelAttack1, epos, 70, PROJECTILE_SPRITE_BLUE);
            if not enemy_valid(e) then return end;
            t_wait(1.5)
            async_task_lambda(SubBoss23_UnravelAttack1, epos, -60, PROJECTILE_SPRITE_PURPLE);
            async_task_lambda(SubBoss23_UnravelAttack1, epos, 60, PROJECTILE_SPRITE_RED);
            if not enemy_valid(e) then return end;
            t_wait(0.90)
            async_task_lambda(SubBoss23_UnravelAttack1, epos, -80, PROJECTILE_SPRITE_RED);
            t_wait(0.20)
            async_task_lambda(SubBoss23_UnravelAttack1, epos, 80, PROJECTILE_SPRITE_PURPLE);
            if not enemy_valid(e) then return end;
            t_wait(1.2)
            async_task_lambda(SubBoss23_UnravelAttack1, epos, -80, PROJECTILE_SPRITE_RED);
            t_wait(0.10)
            async_task_lambda(SubBoss23_UnravelAttack1, epos, 80, PROJECTILE_SPRITE_PURPLE);

            t_wait(3.5);

            -- Sprinkler with some light attacks
            async_task_lambda(SubBoss23_Sprinkler1_AttackPattern, e);
            t_wait(5.5);
         end

         Game_Spawn_Stage2_3_SubBoss1();
         async_task_lambda(_Boss_Synchronized_Logic);
         summoned_next = true;

         enemy_set_acceleration(e, -120, 0);
         t_wait(1.0);
         enemy_set_acceleration(e, 120, 0);
         t_wait(1.0);
         enemy_set_acceleration(e, 0, 0);
         enemy_set_velocity(e, 0, 0);

         boss_state_0.phase = 1;
      elseif boss_state_0.phase == 1 then
         -- Sync boss mode, run BossLogic_Synced
      elseif boss_state_0.phase == 2 then
         -- Last Stand Patterns when returning to center
         print("Take Over 0");
         _Boss_TakeOver0(e);
         print("End Take Over 0");
         boss_state_0.phase = 3;
      elseif boss_state_0.phase == 3 then

      end
      t_yield();
   end

   --... I'm not sure when this could happen because it should be impossible to
   -- do this, but yeah.
   if summoned_next == false then
      Game_Spawn_Stage2_3_SubBoss1();
   end
end

function Game_Spawn_Stage2_3_SubBoss0()
   local e = enemy_new();
   local initial_boss_position = v2(play_area_width()/2, -50);

   enemy_set_hp(e, BOSS0_HP);
   enemy_set_position(e, initial_boss_position[1], initial_boss_position[2]);
   enemy_set_scale(e, 50, 48);
   t_wait(1.0);

   enemy_set_burst_gain_value(e, 0.215);
   enemy_show_boss_hp(e, "BONEWINGS");
   async_task_lambda(_Boss_Logic0, e);
   boss_state_0.me = e;
   boss_state_0.last_good_position = initial_boss_position;
   boss_state_0.phase = 0;
end

-- BOSS 2
function _Boss_Intro1(e)
   enemy_begin_invincibility(e, true, 2.5);
   enemy_set_acceleration(e, 120, 120);
   t_wait(1.0);
   enemy_set_acceleration(e, -120, -120);
   t_wait(1.0);
   enemy_set_acceleration(e, 0, 0);
   enemy_set_velocity(e, 0, 0);
end

function _Boss_TakeOver1(e)
   enemy_set_acceleration(e, -120, 0);
   t_wait(1.0);
   enemy_set_acceleration(e, 120, 0);
   t_wait(1.0);
   enemy_set_acceleration(e, 0, 0);
   enemy_set_velocity(e, 0, 0);
end

function _Boss_Logic1(e)
   _Boss_Intro1(e);

   while enemy_valid(e) do
      local epos = enemy_final_position(e);
      if boss_state_1.phase == 0 then
         -- Sync boss mode, run BossLogic_Synced
      elseif boss_state_1.phase == 1 then
         print("Take Over 1");
         _Boss_TakeOver1(e);
         print("End Take Over 1");
         boss_state_1.phase = 2;
      elseif boss_state_1.phase == 2 then
      end
      t_yield();
   end
end

-- Control both, and use specific patterns for both bosses
-- Boss Sync Phase
function _Boss_Synchronized_Logic()
   local e = boss_state_0.me;
   local e1 = boss_state_1.me;
   
   t_wait(3.5);

   -- Copy and paste here since the semantics for this kind of thing
   -- don't really exist in the language...
   while enemy_valid(e) and enemy_valid(e1) do
      if not (enemy_valid(e) and enemy_valid(e1)) then
         break;
      end
      async_task_lambda(SubBoss23_Sprinkler2_AttackPattern, e);
      if not (enemy_valid(e) and enemy_valid(e1)) then
         break;
      end
      async_task_lambda(SubBoss23_Sprinkler2_AttackPattern, e1);
      if not (enemy_valid(e) and enemy_valid(e1)) then
         break;
      end
      t_wait(5.5);
      if not (enemy_valid(e) and enemy_valid(e1)) then
         break;
      end
      t_wait(5.65);
      if not (enemy_valid(e) and enemy_valid(e1)) then
         break;
      end
      async_task_lambda(SubBoss23_Sprinkler1_AttackPattern, e);
      if not (enemy_valid(e) and enemy_valid(e1)) then
         break;
      end
      t_wait(5.0);
      if not (enemy_valid(e) and enemy_valid(e1)) then
         break;
      end
      t_wait(5.65);
      if not (enemy_valid(e) and enemy_valid(e1)) then
         break;
      end
      async_task_lambda(SubBoss23_Sprinkler1_AttackPattern, e1);
      if not (enemy_valid(e) and enemy_valid(e1)) then
         break;
      end
      t_wait(5.65);

      t_yield()
   end

   -- Advance the survivor to their single boss phase

   if enemy_valid(e) then
      boss_state_0.phase = 2;
   end

   if enemy_valid(e1) then
      boss_state_1.phase = 1;
   end
end

function Game_Spawn_Stage2_3_SubBoss1()
   local e = enemy_new();
   local initial_boss_position = v2(play_area_width()/2, -50);

   enemy_set_hp(e, BOSS1_HP);
   enemy_set_position(e, initial_boss_position[1], initial_boss_position[2]);
   enemy_set_scale(e, 50, 48);
   t_wait(1.0);

   enemy_set_burst_gain_value(e, 0.215);
   enemy_show_boss_hp(e, "BONEWINGS");
   async_task_lambda(_Boss_Logic1, e);
   boss_state_1.me = e;
   boss_state_1.last_good_position = initial_boss_position;
   boss_state_1.phase = 0;
end

function Game_Spawn_Stage2_3_SubBoss()
   -- play_area_set_edge_behavior(PLAY_AREA_EDGE_TOP, PLAY_AREA_EDGE_BLOCKING);
   -- play_area_set_edge_behavior(PLAY_AREA_EDGE_BOTTOM, PLAY_AREA_EDGE_BLOCKING);
   -- play_area_set_edge_behavior(PLAY_AREA_EDGE_LEFT, PLAY_AREA_EDGE_WRAPPING);
   -- play_area_set_edge_behavior(PLAY_AREA_EDGE_RIGHT, PLAY_AREA_EDGE_WRAPPING);
   Game_Spawn_Stage2_3_SubBoss0();
   -- Game_Spawn_Stage2_3_SubBoss1();
end
