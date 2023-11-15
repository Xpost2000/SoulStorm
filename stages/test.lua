--function enemy11_a(e)
--  enemy_set_velocity(e, enemy_velocity_x(), 250);
--  t_wait(0.55);
--  enemy_reset_movement(e);
--  while (enemy_valid(e)) do
    
--    spawn_bullet_arc_pattern2(
--    e_position_x(), e_position_y(), 
--    5, 45, 
--    5, 5, 
--    0, 1, 
    
--    1000, 
    
--    0, 
    
--    BULLET_SOURCE_ENEMY, 
--    PROJECTILE_SPRITE_RED_ELECTRIC);
    
--    t_wait(0.25)
--  end
--end

function stage_task()
  t_wait(0.5);

  for i=0, 3 then
    local e = enemy_new();
    enemy_set_position(e, i*35, -30);
    enemy_set_scale(e, 10, 10);
    enemy_set_hp(e, 10);
    enemy_set_velocity(e, 0, 125);
    
    local e2 = enemy_new();
    enemy_set_position(2, play_area_width() -  i*35, -30);
    enemy_set_scale(e2, 10, 10);
    enemy_set_hp(e2, 10);
    enemy_set_velocity(e2, 0, 125);
  end
  
  t_wait(1.0)
  
--  for i=0,10 then
--    local e = enemy_new();
--    enemy_set_position(e, i*35, -30);
--    enemy_set_scale(e, 10, 10);
--    enemy_set_hp(e, 10);
--    enemy_set_velocity(e, 0, 155);
--  end
--  t_wait(1.0);

--  for i=0, 5 then
--    local e = enemy_new();
--    enemy_set_position(e, i*35, -30);
--    enemy_set_scale(e, 10, 10);
--    enemy_set_hp(e, 10);
    
--    if i == 3 then
--      enemy_set_task(e, "enemy11_a");
--    else
--      enemy_set_velocity(e, 0, 100);
--    end
--    t_wait(0.25);
--  end
--  while any_living_danger() do
--    t_yield();
--  end
  t_complete_stage();
end