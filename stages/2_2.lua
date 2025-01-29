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

   wait_no_danger();
   t_complete_stage();
end
