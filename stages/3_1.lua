engine_dofile("stages/common.lua")

function stage_task()
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage3bkg1_0.png", 0.28, 0, 0);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star0.png", 0.487, 225, 85);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star1.png", 0.287, -120, 175);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.187, 355, 15);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.117, -150, 255);
   wait_no_danger();
   t_complete_stage();
end
