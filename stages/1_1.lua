dofile("stages/common.lua")

function enemy11_a(e, a, b)
    print("--------")
    print(a);
    print(b);
    print("--------")
  print(e);
  print("Hello?")
  enemy_set_velocity(e, enemy_velocity_x(e), 150);
  t_wait(0.55);
  print("Let's get started.");
  enemy_reset_movement(e);

  local i = 0;
  while (enemy_valid(e)) and i < 3 do
    t_wait(0.1)

    spawn_bullet_arc_pattern2(
    enemy_position_x(e), enemy_position_y(e), 
    5, 45, 
    5, 5, 
    0, 1, 
    
    175, 
    
    0, 
    
    BULLET_SOURCE_ENEMY, 
    PROJECTILE_SPRITE_RED_ELECTRIC);
    
    t_wait(0.25)
    i = i+1;
  end
  print("Bye.");
  enemy_set_acceleration(e, -150, 120);
end

function enemy11_b(e)
  print(e);
  print("Hello?")
  enemy_set_velocity(e, enemy_velocity_x(e), 150);
  t_wait(0.55);
  print("Let's get started.");
  enemy_reset_movement(e);

  local i = 0;
  while (enemy_valid(e)) and i < 3 do
    t_wait(0.1)

    spawn_bullet_arc_pattern2(
    enemy_position_x(e), enemy_position_y(e), 
    5, 45, 
    5, 5, 
    0, 1, 
    
    225, 
    
    0, 
    
    BULLET_SOURCE_ENEMY, 
    PROJECTILE_SPRITE_RED_ELECTRIC);
    
    t_wait(0.25)
    i = i+1;
  end
  print("Bye.");
  enemy_set_acceleration(e, 150, 120);
end

function test_stagger_spawn()
  for i=0, 10 do
    local e = enemy_new();
    enemy_set_position(e, i*35, -30);
    enemy_set_scale(e, 10, 10);
    enemy_set_hp(e, 10);
    
    if i == 3 then
      enemy_set_task(e, "enemy11_a");
    elseif i == 7 then
      enemy_set_task(e, "enemy11_b");
    else
      enemy_set_velocity(e, 0, 100);
    end
    
    t_wait(0.25);
  end
end

function test_stagger_spawn2()
  for i=0, 10 do
    local e = enemy_new();
    enemy_set_position(e, i*35, -30);
    enemy_set_scale(e, 10, 10);
    enemy_set_hp(e, 10);
    
    if i % 2 == 0 then
      enemy_set_task(e, "enemy11_a");
    else
      enemy_set_task(e, "enemy11_b");
    end
    
    t_wait(0.25);
  end
end

function spinning_bullet_task(b)
         for i = 0 , 90 do
            local direction = v2_direction_from_degree(i);
            local x = direction[1] * 50;
            local y = direction[2] * 50;
            bullet_set_velocity(b, x, y);
            t_wait(0.01)
         end
         bullet_set_velocity(b, 0, 200);
         bullet_start_trail(b, 16);
         bullet_set_trail_record_speed(b, 0.085);
end

function stage_task()
  print(common);
   t_wait(0.5);

     local e = enemy_new();
     enemy_set_position(e, 100, 50);
     enemy_set_scale(e, 10, 10);
     enemy_set_hp(e, 1000);
     -- enemy_set_velocity(e, 0, 155);
     enemy_show_boss_hp(e, "God");

   t_wait(1.5);
     local e1 = enemy_new();
     enemy_set_position(e1, 100, 100);
     enemy_set_scale(e1, 10, 10);
     enemy_set_hp(e1, 1000);
     enemy_show_boss_hp(e1, "God");
 
    print("hi")
    print(_spawn_bullet_arc_pattern2);
    _spawn_bullet_arc_pattern2(v2(50, 50), 15, 45, v2(0, 1), 100, 45)
    print("bye")
--   for i=0, 3 do
--     local e = enemy_new();
--     enemy_set_position(e, i*35, -30);
--     enemy_set_scale(e, 10, 10);
--     enemy_set_hp(e, 10);
--     enemy_set_velocity(e, 0, 125);
--     
--     local e2 = enemy_new();
--     enemy_set_position(e2, play_area_width() -  i*35, -30);
--     enemy_set_scale(e2, 10, 10);
--     enemy_set_hp(e2, 10);
--     enemy_set_velocity(e2, 0, 125);
--   end
--   
--   t_wait(1.0)
--   
--   for i=0,10 do
--     local e = enemy_new();
--     enemy_set_position(e, i*35, -30);
--     enemy_set_scale(e, 10, 10);
--     enemy_set_hp(e, 10);
--     enemy_set_velocity(e, 0, 155);
--   end
--   t_wait(1.0);
-- 
--   test_stagger_spawn()
--   t_wait(2.0);
--   test_stagger_spawn2()
   
   while any_living_danger() do
     t_yield();
   end
   t_wait(10.0);
   t_complete_stage();
 end