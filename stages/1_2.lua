dofile("stages/common.lua")
function wave_1()
   do
      local e = enemy_new();
      -- testing movement behaviors
      enemy_set_position(e, play_area_width()/2, play_area_height()/2-100);
      enemy_task_lambda(
         e,
         function (e)
            enemy_set_velocity(e, 0, 30);
            t_wait(0.5);
            enemy_set_acceleration(e, 0, 35);
            t_wait(1.5);
            local st = enemy_time_since_spawn(e);

            while true do
               local dt = enemy_time_since_spawn(e) - st;

               local v = enemy_velocity(e);
               enemy_set_velocity(e, v[1] * 0.8, v[2] * 0.8);

               if dt > 0.5 then
                  enemy_reset_movement(e);
                  break;
               end

               t_yield();
            end
            print('bye');
         end
      )
   end
end

function stage_task()
   t_wait(5);
   wave_1();
   t_wait(5);
   wait_no_danger();
   t_complete_stage();
end
