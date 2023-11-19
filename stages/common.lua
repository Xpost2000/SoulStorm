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
function bullet_list_set_scale(bullets, scale)
   for i,b in ipairs(bullets) do
      bullet_set_scale(b, scale[1], scale[2]);
   end
end

function spawn_bullet_line(center, how_many, spacing, scale, direction, speed)
   local new_bullets = {};
   local direction     = v2_normalized(direction);
   local pattern_width = spacing * how_many + scale[1] * how_many;
   local perpendicular_direction = v2_perp(direction);

   -- No +=/-= hurts :/
   center[1] = center[1] - (perpendicular_direction[1] * (pattern_width/2));
   center[2] = center[2] - (perpendicular_direction[2] * (pattern_width/2));

   bullet_i = 1;

   for i=0, how_many do
      local nb = bullet_new();
      new_bullets[bullet_i] = nb;
      bullet_set_position(nb, center[1], center[2]);
      bullet_set_velocity(nb, direction[1] * speed, direction[2] * speed);
      bullet_set_scale(nb, scale[1], scale[2])
      bullet_i = bullet_i + 1;

      center[1] = center[1] + perpendicular_direction[1] * (spacing + scale[1])
      center[2] = center[2] + perpendicular_direction[2] * (spacing + scale[1])
   end

   return new_bullets;
end

function spawn_bullet_arc_pattern2(center, how_many, arc_degrees, direction, speed, distance_from_center, src)
   local new_bullets = {};
   local arc_sub_length = arc_degrees / how_many;
   local direction_angle = math.deg(math.atan(direction[2], direction[1]));
   
   local bullet_i = 1;
   
   for i=0, how_many do
      local angle = direction_angle + arc_sub_length * (i - (how_many/2));
      local current_arc_direction = v2_direction_from_degree(angle);
      local position = v2_add(center, v2(current_arc_direction[1] * distance_from_center, current_arc_direction[2] * distance_from_center));
      
      local nb = bullet_new(src);
      new_bullets[bullet_i] = nb;
      bullet_set_position(nb, position[1], position[2]);
      bullet_set_scale(nb, 10, 10);
      bullet_set_velocity(nb, current_arc_direction[1] * speed, current_arc_direction[2] * speed);
      bullet_set_lifetime(nb, 10);
      bullet_i = bullet_i + 1;
   end
   return new_bullets;
end

function wait_no_danger()
      while any_living_danger() do
         t_yield();
      end
end

function enemy_linear_move_to(e, x, y, t)
   local cur_x = enemy_position_x(e);
   local cur_y = enemy_position_y(e);

   local dx = (x - cur_x) / t;
   local dy = (y - cur_y) / t;

   enemy_set_velocity(e, dx, dy);
   t_wait(t);
   enemy_reset_movement(e);
end

