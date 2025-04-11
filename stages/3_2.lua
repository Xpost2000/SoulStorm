engine_dofile("stages/common.lua")

function stage_task()
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage3bkg2_0.png", 0.50, 0, 0);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star0.png", 0.117, 225, 125);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg1_star1.png", 0.147, -150, 245);

   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.27, 100, 65);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.37, 500, 200);

   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage2bkg2_1.png", 0.180, 0, 125);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage1bkg2_1_2.png", 0.150, 0, 305);
   wait_no_danger();
   t_complete_stage();
end
