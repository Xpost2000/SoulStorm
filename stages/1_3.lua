engine_dofile("stages/common.lua")

function stage_task()
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_0.png", 0.25, 0, -25);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.187, 355, 15);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.117, -150, 255);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.450, 35, 0);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.800, 85, 200);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.650, 450, 200);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.750, 100, 377);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.800, -70, 200);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.552, 90, 50);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/boss_0_light_streak.png", 0.150, 0, 125);

   -- wait_no_danger();
   -- explosion_hazard_new(100, 100, 50, 0.5, 2);
   -- explosion_hazard_new(100, 180, 100, 0.5, 1);
   laser_hazard_new(100, 20, 0, 0.5, 1.5);
   laser_hazard_new(100, 20, 1, 0.5, 1.5);
   laser_hazard_new(200, 20, 1, 0.5, 1.5);
   -- t_complete_stage();
end
