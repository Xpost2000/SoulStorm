engine_dofile("stages/common.lua")

function stage_task()
   -- wait_no_danger();
   -- explosion_hazard_new(100, 100, 50, 0.5, 2);
   -- explosion_hazard_new(100, 180, 100, 0.5, 1);
   laser_hazard_new(100, 20, 0, 0.5, 1.5);
   laser_hazard_new(100, 20, 1, 0.5, 1.5);
   laser_hazard_new(200, 20, 1, 0.5, 1.5);
   -- t_complete_stage();
end
