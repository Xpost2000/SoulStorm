dofile("stages/common.lua")
function stage_task()
   t_wait(5);
   wait_no_danger();
   t_complete_stage();
end
