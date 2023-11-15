function bullet_list_set_visuals(bullets, visual)
  for i,b in ipairs(bullets) do
    bullet_set_visual(b, visual);
  end
end
function bullet_list_set_source(bullets, source)
  for i,b in ipairs(bullets) do
    bullet_set_source(b, source);
  end
end

function _spawn_bullet_arc_pattern2(center, how_many, arc_degrees, direction, speed, distance_from_center)
  local new_bullets = {};
  local arc_sub_length = arc_degrees / how_many;
    print("Before direction angle")
  local direction_angle = math.deg(math.atan(direction[2], direction[1]));
  
  local bullet_i = 1;
  
  print("Before bullet spawn")
  
  for i=0, how_many do
    local angle = direction_angle + arc_sub_length * (i - (how_many/2));
    local current_arc_direction = v2_direction_from_degree(angle);
    print("First time");
    
        current_arc_direction[1] = current_arc_direction[1] * distance_from_center;
    current_arc_direction[2] = current_arc_direction[2] * distance_from_center;
    local position = v2_add(center, current_arc_direction);
  
    new_bullets[bullet_i] = bullet_new(BULLET_SOURCE_ENEMY);
    bullet_set_position(new_bullets[bullet_i], position[1], position[2]);
    bullet_set_scale(new_bullets[bullet_i], 10, 10);
    bullet_set_velocity(new_bullets[bullet_i], current_arc_direction[1] * speed, current_arc_direction[2] * speed);
    bullet_set_lifetime(new_bullets[bullet_i], 10);
    print("Make bullet pls")  
    print(new_bullets[bullet_i]);
    bullet_i = bullet_i + 1;
  end
  return new_bullets;
end

print("Hello, common.lua!")
print(deg)
print(math)
print(math.deg)
print(_spawn_bullet_arc_pattern2)