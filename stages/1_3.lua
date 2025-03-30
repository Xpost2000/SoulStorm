engine_dofile("stages/common.lua")
engine_dofile("stages/boss1.lua");

enable_boss_prewave = true;
enable_boss_intro_explosions = true;

local track0 = load_music("res/snds/music_1_3_preboss.ogg");

function wave1()
   Stage1_Batflood();
   t_wait(1)
   do
      local rx = 50 - 10;
      local ry = 50 + 10;
      local e0 = Make_Enemy_Spinner_1_1_2(25, v2(45, -30), v2(0, 1), 200, 0.5, 0.5, 25, rx, ry, PROJECTILE_SPRITE_CAUSTIC_DISK);
      local e1 = Make_Enemy_Spinner_1_1_2(25, v2(play_area_width()-45, -30), v2(0, 1), 200, 0.5, 0.5, 25, rx, ry, PROJECTILE_SPRITE_CAUSTIC_DISK);
      enemy_set_visual(e0, ENTITY_SPRITE_SKULL_A1);
      enemy_set_visual(e1, ENTITY_SPRITE_SKULL_A1);
   end
   LaserChaser_Horizontal_1_2(4, 1.25);
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

         PROJECTILE_SPRITE_CAUSTIC_STROBING,
         4
      );
      enemy_set_visual(e0, ENTITY_SPRITE_SKULL_A1);

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

         PROJECTILE_SPRITE_CAUSTIC_STROBING,
         4
      );
      enemy_set_visual(e1, ENTITY_SPRITE_SKULL_A1);
   end
   t_wait(2);
   LaserChaser_Vertical_1_2(4, 1.25);
   t_wait(3);
   Stage1_Batflood();
   t_wait(2.5);
   do
      for i=1,6 do
         do
            local p0 = PROJECTILE_SPRITE_RED_ELECTRIC;
            local p1 = PROJECTILE_SPRITE_RED_DISK;
            if i % 2 == 0 then
               p0 = PROJECTILE_SPRITE_BLUE_STROBING;
               p1 = PROJECTILE_SPRITE_BLUE_DISK;
            end
            local e0 = Make_Enemy_SideMoverWave1_1_1(10 + i * 35, -100, 0, 100, 1.5, 1, 0, 12, p0, p1);
            enemy_set_visual(e0, ENTITY_SPRITE_BAT_B1);
            if i % 2 == 0 then
               t_wait(0.75);
            else
               t_wait(1.0);
            end
         end
      end
   end
   -- right
   Make_BrainDead_Enemy_Popcorn1(
      20,
      v2(play_area_width() + 25, player_position_y()),
      0.086,
      7,
      -150,
      30,
      2,
      25,
      4,
      -1);
   t_wait(2.5);
   --left
   Make_BrainDead_Enemy_Popcorn1(
      32,
      v2(-30, player_position_y()),
      0.14,
      5,
      150,
      -10,
      30,
      25,
      2,
      2
   );
   t_wait(3);
   for i=1,8 do
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
         PROJECTILE_SPRITE_PURPLE_DISK,
         4
         );
      -- NOTE(jerry): make new bat sprites for these things
      enemy_set_visual(e, ENTITY_SPRITE_SKULL_B1);
   end
   t_wait(2);
end

function boss_intro_wave()
   if enable_boss_intro_explosions then
      start_black_fade(0.065);

      explosion_hazard_new(50, 50, 25, 0.25, 0.25);
      explosion_hazard_new(play_area_width()-50, 50, 25, 0.25, 0.25);
      t_wait(0.55);
      explosion_hazard_new(75, 65, 35, 0.10, 0.10);
      t_wait(0.15);
      explosion_hazard_new(play_area_width()-95, 45, 35, 0.10, 0.10);
      t_wait(0.15);
      explosion_hazard_new(110, 25, 35, 0.10, 0.10);
      t_wait(0.15);
      explosion_hazard_new(play_area_width()-100, 25, 35, 0.10, 0.10);
      t_wait(0.15);
      explosion_hazard_new(140, 45, 35, 0.15, 0.15);
      t_wait(0.35);
      explosion_hazard_new(75, 65, 35, 0.10, 0.10);
      t_wait(0.25);
      explosion_hazard_new(play_area_width()-95, 45, 35, 0.10, 0.10);
      t_wait(0.25);
      explosion_hazard_new(110, 25, 35, 0.10, 0.10);
      t_wait(1.25);
      explosion_hazard_new(play_area_width()/2+25, 35, 35, 0.10, 0.10);
      explosion_hazard_new(play_area_width()/2-50, 35, 35, 0.10, 0.10);
      explosion_hazard_new(play_area_width()/2-35, 45, 35, 0.10, 0.10);
      explosion_hazard_new(play_area_width()/2, 85, 35, 0.10, 0.10);
      explosion_hazard_new(play_area_width()/2, 50, 65, 0.125, 0.25);
      t_wait(2.0);
   end
   local b = Game_Spawn_Stage1_Boss();
   --local e = Game_Spawn_Stage1_Boss_HexBind0();
   --local y = Game_Spawn_Stage1_Boss_HexBind1();
   enemy_begin_invincibility(b, true, 99999);
   t_wait(3.5);
   -- boss spawn!!
   end_black_fade();
   enemy_end_invincibility(b);
end

function stage_task()
   play_area_set_edge_behavior(PLAY_AREA_EDGE_TOP, PLAY_AREA_EDGE_DEADLY);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_BOTTOM, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_LEFT, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_RIGHT, PLAY_AREA_EDGE_BLOCKING);
   play_music(track0);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_0.png", 0.25, 0, -25);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.187, 355, 15);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.117, -150, 255);
   -- Disabled because it makes the boss fight too difficult to read.
   -- Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.450, 35, 0);
   -- Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.800, 85, 200);
   -- Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.650, 450, 200);
   -- Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.750, 100, 377);
   -- Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.800, -70, 200);
   -- Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.552, 90, 50);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/boss_0_light_streak.png", 0.150, 0, 125);

   if enable_boss_prewave then
    t_wait(1.5);
    wave1();
    t_wait(12.5);
    convert_all_bullets_to_score()
    t_wait(3.5);
    LaserChaser_Horizontal_1_2(4, 1.25);
    t_wait(1.0);
   end

   -- Game_Spawn_Stage1_Boss_HexBind0()
   boss_intro_wave();
   
  -- t_complete_stage();
end
