dofile("stages/common.lua")

--[[
   Stage 1 - 1

   Expected Length : 3.5 minutes?
   Difficulty      : Easy/Medium? Casual

   Author: xpost2000/Jerry Zhu
]]--

--[[
   Wave 1
   Expected Length: 35 seconds?
]]
function wave_1()
   --[[
      two sub encounters.
   ]]--
   t_wait(1.55)
end

function stage_task()
   wave_1();
   t_complete_stage();
end
