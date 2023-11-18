dofile("stages/common.lua")

local testmusic = load_music("res/snds/8bitinternetoverdose_placeholder.ogg");

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
function loop_bkg_music()
   while true do
      if not music_playing() then
         play_music(testmusic);
      end
      t_yield();
   end
end
function wave_1()
   --[[
      two sub encounters.
   ]]--
   t_wait(1.55)
end

function stage_task()
   async_task("loop_bkg_music");
   wave_1();
   t_complete_stage();
end
