-- final boss code, meant to be included in stage file

local track1 = load_music("res/snds/music_1_3_bossloop.ogg"); -- todo replace

enable_boss_death_explosion = false;
BOSS_HP = 8350;

boss_teleport_positions = {}; -- forward declaration
boss_state = {
    me,
    last_good_position, -- for if the entity is deleted for any reason
    starting_position,

    -- general boss logic state
    next_rain_attack_until = -9999,

    -- Attacking actions. Attack actions and movements
    -- are scheduled "simulatneously"
    next_think_action_t = -9999,
    next_grid_action_t = -9999,

    -- Not always moving, but sometimes move...
    next_possible_move_action_t = -9999,

    -- phase 1 : shadow play
    -- phase 2 : shadow WITCH
    -- phase 3 : shadow play
    -- phase 4 : shadow bone wings
    -- phase 5 : shadow play
    -- phase 6 : shadow boss 2
    -- phase 7..9 : more erratic
    phase=0,

    -- movement data
    wants_to_move=false,
    moving_to=false,
    move_target,
    move_method='teleport',
    move_speed, -- Velocity if not teleporting
    -- explosion speed is fixed size
};

function _Stage_Boss_MusicPlayer(e)
   while enemy_valid(e) do
      if (music_playing() == false) then
         play_music(track1);
      end
      t_yield();
   end
end

-- sets the animation state machine
function _Stage_Boss_Animator(e)
    while enemy_valid(e) do
        local position = v2(enemy_position_x(e), enemy_position_y(e));
        local velocity = v2(enemy_velocity_x(e), enemy_velocity_y(e));
        local magnitude = v2_magnitude(velocity);
        local particle_emitter_ptr = enemy_get_particle_emitter(boss_state.me, 0);

        -- particles are "animated"
        particle_emitter_set_active(particle_emitter_ptr, true);
        particle_emitter_set_lifetime(particle_emitter_ptr, 1.25);
        particle_emitter_set_scale(particle_emitter_ptr, 0.73);
        particle_emitter_set_max_emissions(particle_emitter_ptr, -1);
        particle_emitter_set_emit_per_emission(particle_emitter_ptr, 4);
        particle_emitter_set_emission_max_timer(particle_emitter_ptr, 0.01);
        particle_emitter_set_acceleration(particle_emitter_ptr, 0, 98);
        particle_emitter_set_velocity_x_variance(particle_emitter_ptr, -64, 64);
        particle_emitter_set_velocity_y_variance(particle_emitter_ptr, -64, 64);
        particle_emitter_set_scale_variance(particle_emitter_ptr, 0.02, 0.1);
        particle_emitter_set_emit_shape_circle(particle_emitter_ptr, position[1], position[2]+25, 8, false);
        particle_emitter_set_sprite_projectile(particle_emitter_ptr, PROJECTILE_SPRITE_SPARKLING_STAR);

        if (magnitude < 4) then
            enemy_set_visual(e, ENTITY_SPRITE_BOSS3_FACE_FRONT);
        else
            if velocity[1] > 0 then
                enemy_set_visual(e, ENTITY_SPRITE_BOSS3_LEAN_RIGHT);
            elseif velocity[1] < 0 then
                enemy_set_visual(e, ENTITY_SPRITE_BOSS3_LEAN_LEFT);
            end
        end
        t_yield();
    end
end

function _Stage_Boss_Thinker(e)
    ---  enemy_set_velocity(e, -100, 40);
    ---  t_wait(2);
    ---  enemy_set_velocity(e, 250, 60);
    ---  t_wait(1);
    ---  enemy_set_velocity(e, 0, 0);
end

function _Stage_Boss_Spoke_Of_The_WheelAttack(e)
    disable_bullet_to_points();
    local running = true;
    local radius_buildup=1;
    start_black_fade(0.05);
    async_task_lambda(function(e)
        while running and enemy_valid(e) do
            local position = v2(enemy_position_x(e), enemy_position_y(e));
            -- particles are "animated"
            local particle_emitter_ptr = enemy_get_particle_emitter(boss_state.me, 1);
            particle_emitter_set_active(particle_emitter_ptr, true);
            particle_emitter_set_lifetime(particle_emitter_ptr, 0.15);
            particle_emitter_set_scale(particle_emitter_ptr, 1);
            particle_emitter_set_max_emissions(particle_emitter_ptr, -1);
            particle_emitter_set_emit_per_emission(particle_emitter_ptr, 128);
            particle_emitter_set_emission_max_timer(particle_emitter_ptr, 0.01);
            particle_emitter_set_acceleration(particle_emitter_ptr, 0, 98);
            particle_emitter_set_velocity_x_variance(particle_emitter_ptr, -64, 64);
            particle_emitter_set_velocity_y_variance(particle_emitter_ptr, -64, 64);
            particle_emitter_set_scale_variance(particle_emitter_ptr, 0.02, 0.1);
            particle_emitter_set_emit_shape_circle(particle_emitter_ptr, position[1], position[2], radius_buildup, false);
            particle_emitter_set_sprite_projectile(particle_emitter_ptr, PROJECTILE_SPRITE_SPARKLING_STAR);
            t_yield();
        end
    end, e);

    while enemy_valid(e) and running do
        local position = v2(enemy_position_x(e), enemy_position_y(e));
        local velocity = v2(enemy_velocity_x(e), enemy_velocity_y(e));

        local spokes = 16;
        local spoke_angle = 360 / spokes;
        local preturn = 30;
        local laser_speed= 7200;

        while radius_buildup < 48 do
            radius_buildup = radius_buildup + 1.5;
            if radius_buildup > 48 then
                radius_buildup = 48;
            end
            t_yield();
        end

        function create_spoke(turn)
            -- fake engine lasers, cause I don't have real ones
            -- on the brightside, it means we can attack the laser spiral
            for j=4,10 do
                for i=0,spokes do
                    local ang = spoke_angle * i + turn;
                    do
                        local bullet = bullet_new(BULLET_SOURCE_ENEMY);
                        local fire_dir = v2_direction_from_degree(ang);

                        bullet_set_position(bullet, position[1]+fire_dir[1]*16*j, position[2]+fire_dir[2]*16*j);
                        bullet_set_visual(bullet, PROJECTILE_SPRITE_LASER_PURPLE);
                        bullet_set_lifetime(bullet, 0.75);
                        bullet_set_scale(bullet, 5, 5);
                        bullet_set_visual_scale(bullet, 1, 1);

                        bullet_set_velocity(bullet, fire_dir[1] * laser_speed, fire_dir[2] * laser_speed);
                    end
                end
            end
        end

        for adj=0,120 do
            preturn = preturn + 0.2;
            create_spoke(preturn);
            t_yield();
        end
        
        for adj=0,240 do
            preturn = preturn - 1.1;
            create_spoke(preturn);
            t_yield();
        end

        running = false;
        break;
    end
    end_black_fade(0.05);
end

function Game_Spawn_Stage_Boss()
   local e = enemy_new();
   local initial_boss_pos = v2(play_area_width()/2, 200);
   enemy_set_hp(e, BOSS_HP); -- TODO for now
   enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
   enemy_set_visual(e, ENTITY_SPRITE_BOSS3_FACE_FRONT);
   enemy_set_burst_gain_value(e, 0.55);
   enemy_show_boss_hp(e, "???");

   -- The boss takes "three threads" of logic.
   -- async_task_lambda(_Stage1_Boss_Logic, e);
   -- async_task_lambda(_Stage1_Boss_Maintain_Hexes_Logic, e);
   -- async_task_lambda(_Stage1_Boss_ImmunityToBurstLaser_Logic, e);
   async_task_lambda(_Stage_Boss_MusicPlayer, e);
   async_task_lambda(_Stage_Boss_Animator, e);
   async_task_lambda(_Stage_Boss_Thinker, e);

   boss_state.me = e;
   boss_state.starting_position = initial_boss_pos;

   t_wait(1);
   async_task_lambda(_Stage_Boss_Spoke_Of_The_WheelAttack, e);

   return e;
end