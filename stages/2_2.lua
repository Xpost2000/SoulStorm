engine_dofile("stages/common.lua")

-- TODO SKULL C
function wave1()
   DramaticExplosion_SpawnSpinnerObstacle1_2_1(
      play_area_width()/2,
      play_area_height()/2,
      ENTITY_SPRITE_SKULL_A,
      12,
      30,
      PROJECTILE_SPRITE_GREEN_DISK,
      v2(0, 1),
      0
   );
   t_wait(1);
   
   DramaticExplosion_SpawnSpinnerObstacle1_2_1(
      20,
      play_area_height()-20,
      ENTITY_SPRITE_SKULL_B,
      -9,
      30,
      PROJECTILE_SPRITE_RED,
      v2(-1, -1),
      5
   );
   t_wait(2);
   
   DramaticExplosion_SpawnSpinnerObstacle1_2_1(
      play_area_width()-20,
      20,
      ENTITY_SPRITE_SKULL_B,
      -9,
      30,
      PROJECTILE_SPRITE_RED,
      v2(-1, -1),
      5
   );

   t_wait(6);
   do
      for i=1,5 do
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
            PROJECTILE_SPRITE_BLUE_DISK,
            4
            );
         -- NOTE(jerry): make new bat sprites for these things
         enemy_set_visual(e, ENTITY_SPRITE_BAT_B);
      end

      t_wait(4);
      do
         Make_BrainDead_Enemy_Popcorn1(
            45,
            v2(play_area_width() + 40, play_area_height()/2),
            0.065,
            5,
            -150,
            -5,
            5,
            5,
            2,
            -1,
            ENTITY_SPRITE_SKULL_A
         );
      end
      t_wait(8);
      do
         Make_BrainDead_Enemy_Popcorn1(
            45,
            v2(play_area_width() + 40, play_area_height()/2),
            0.065,
            5,
            -150,
            -5,
            5,
            5,
            2,
            -1,
            ENTITY_SPRITE_BAT_B
         );
      end
      t_wait(5)
      convert_all_bullets_to_score();
   end
end

-- will fire an attack aimed at the player in the form of a Helix
-- the helix is just meant to look pretty.
function Make_Enemy_Helix_Turret_2_2(
   hp,
   start_position,
   target_position,
   target_position_lerp_t,

   fire_delay,
   burst_count,

   bullet_velocity,
   exit_direction,
   exit_velocity,
   exit_acceleration,

   helix_height,

   sprite,
   bullet_visual, bullet_visual1
)
local e = enemy_new();
end

function wave2()
   t_wait(2);
   -- new enemy type, I need a new sprite
   
end

function stage_task()
   play_area_set_edge_behavior(PLAY_AREA_EDGE_TOP, PLAY_AREA_EDGE_DEADLY);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_BOTTOM, PLAY_AREA_EDGE_DEADLY);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_LEFT, PLAY_AREA_EDGE_BLOCKING);
   play_area_set_edge_behavior(PLAY_AREA_EDGE_RIGHT, PLAY_AREA_EDGE_BLOCKING);
   -- setup stage background
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg2_0.png", 0.53, 0, 0); -- TODO come up with new design
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg2_nebule.png", 0.307, -250, 120);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg2_nebule.png", 0.157, 250, 360);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star0.png", 0.357, 225, 125);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star1.png", 0.147, -150, 245);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.287, 305, 300);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.207, -100, 225);

   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.27, 100, 65);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.37, 500, 200);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg2_1.png", 0.250, 0, 230);
   play_area_notify_current_border_status();
   t_wait(2);
   wave1();
   wave2();

   wait_no_danger();
   t_complete_stage();
end
