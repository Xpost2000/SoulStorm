dofile("stages/common.lua")

function bkg_task()
   print("Hello background task!");
   local face = load_image("res/img/a.png");
   local fog = load_image("res/img/dumpyfogeffect.png");
   local earth = load_image("res/img/earthtest.png");

   local i = 0;
   local j = 0;
   while true do
      local ro   = render_object_create();
      render_object_set_position(ro, 100, 100 + j);
      render_object_set_img_id(ro, face);
      render_object_set_scale(ro, 100, 100);
      render_object_set_y_angle(ro, i);
      render_object_set_z_angle(ro, 0);
      i = i + 1;

      if j > 300 then
         j = 0
      else
         j = j + 1;
      end

      do 
         local ro1   = render_object_create();
         render_object_set_position(ro1, 0, play_area_height()-130);
         render_object_set_img_id(ro1, earth);
         render_object_set_scale(ro1, play_area_width(), play_area_height());
         render_object_set_modulation(ro1, 0.5, 0.5, 0.5, 1.0);
         render_object_set_src_rect(
            ro1,
            render_object_get_src_rect_x(ro1),
            render_object_get_src_rect_y(ro1) + i,
            64,
            64
         );
         render_object_set_y_angle(ro1, 10);
      end

      do 
         local ro1   = render_object_create();
         render_object_set_position(ro1, 0, play_area_height()-150);
         render_object_set_img_id(ro1, fog);
         render_object_set_scale(ro1, play_area_width(), play_area_height());
         render_object_set_src_rect(
            ro1,
            render_object_get_src_rect_x(ro1),
            render_object_get_src_rect_y(ro1) + i*0.5,
            174,
            174
         );
         render_object_set_y_angle(ro1, 10);
      end
     t_yield();
   end
end

function stage_task()
   print("Make async task?");
   async_task("bkg_task");
   t_wait_for_stage_intro();
   t_wait(0.5);

   t_wait(100.0);
 
   while any_living_danger() do
     t_yield();
   end

   t_wait(10.0);
   t_complete_stage();
 end
