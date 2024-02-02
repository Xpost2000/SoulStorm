engine_dofile("stages/common.lua")

function stage_task()
   -- wait_no_danger();
   explosion_hazard_new(100, 100, 50, 0.5, 2);
   explosion_hazard_new(100, 180, 100, 0.5, 1);
   -- t_complete_stage();
end
