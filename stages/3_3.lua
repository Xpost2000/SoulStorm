engine_dofile("stages/common.lua")
engine_dofile("stages/boss3.lua")

-- intentionally copied from common.lua to have
-- more control over the fading visual, that doesn't affect the general version
g_black_fader_id = -1;
g_black_fade_out_per_tick = 0.15;
g_black_fade_t = 0.0;
g_fade_direction = 0;
g_black_fade_started = false;

function _fade_black_task()
   while g_black_fade_started do
      local lastalpha = g_black_fade_t;
      if g_fade_direction == 0 then
         -- fade in
         if (lastalpha < 1.0) then
            g_black_fade_t = g_black_fade_t + g_black_fade_out_per_tick;
         else
            -- nothing. Wait until we end.
         end
      else
         -- fade out
         if (lastalpha > 0.0) then
            g_black_fade_t = g_black_fade_t - g_black_fade_out_per_tick;
         else
            g_black_fade_started = false;
         end
      end
      render_object_set_modulation(g_black_fader_id, 0, 0, 0, g_black_fade_t * 0.85);
      t_yield();
   end
end

function start_black_fade(fade_speed)
   if not g_black_fade_started then
      if g_black_fader_id == -1 then
         g_black_fader_id = render_object_create();
         render_object_set_layer(g_black_fader_id, SCRIPTABLE_RENDER_OBJECT_LAYER_BACKGROUND);
         render_object_set_img_id(g_black_fader_id, 0);
         render_object_set_scale(g_black_fader_id, 375, 480);
         render_object_set_position(g_black_fader_id, 0, 0);
         render_object_set_modulation(g_black_fader_id, 0, 0, 0, 0.0);
      end
      g_black_fade_out_per_tick = fade_speed;
      g_black_fade_started = true;
      g_fade_direction = 0;
      async_task_lambda(_fade_black_task);
   end
end

function end_black_fade()
   g_fade_direction = 1;
end

function stage_task()
   wait_no_danger();
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_3.png", 0.15, 0, -25);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star0.png", 0.27, 355, 15);
   Generic_Infinite_Stage_ScrollV_FG("res/img/stagebkg/stage1bkg1_star1.png", 0.147, -150, 205);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.450, 35, 0);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.800, 85, 200);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.650, 450, 200);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.750, 100, 377);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.800, -70, 200);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage_boss0_starpattern0.png", 0.552, 90, 50);
   Generic_Infinite_Stage_ScrollV_BG("res/img/stagebkg/stage2bkg2_1.png", 0.150, 0, 125);
   Game_Spawn_Stage_Boss();
end
