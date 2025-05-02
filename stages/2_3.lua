engine_dofile("stages/common.lua")

function stage_task()
   wait_no_danger();
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_2.png", 0.15, 0, -25);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.27, 355, 15);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.147, -150, 205);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.450, 35, 0);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.800, 85, 200);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.650, 450, 200);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.750, 100, 377);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.800, -70, 200);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.552, 90, 50);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/boss_0_light_streak.png", 0.150, 0, 125);
   -- t_complete_stage();
   Game_Spawn_Stage2_2_SubBoss();
end
