SUBBOSS31_HP=3550;
enable_boss_intro=true

subboss31_state = {
   last_good_position,
   me,
   phase
};

function _Boss_Sprout_0(e)
    for i=0,3 do
        for j=0,65 do
            if not enemy_valid(e) then return end
            local pos = enemy_final_position(e);
            local nb = bullet_new(BULLET_SOURCE_ENEMY);
            local vx = math.sin(player_position_x() * 18 +j *10 + 100*i) * 38;
            local vy = 80 + math.sin(player_position_x() * 15 + j *10 + i*100) * 2.25;
            bullet_set_position(nb, pos[1], pos[2]+35);
            bullet_set_visual(nb, PROJECTILE_SPRITE_PURPLE_DISK);
            bullet_set_scale(nb, 1.5, 1.5);
            bullet_set_visual_scale(nb, 0.3, 0.3);
            bullet_set_velocity(nb, vx, vy);
            t_wait(0.016);
        end
    end
end

function _Boss_Sprout_1(e)
    for i=0,5 do
        for j=0,360,40 do
            if not enemy_valid(e) then return end
            local arcdir = v2_direction_from_degree(j);
            local pos = enemy_final_position(e);
            local nb = bullet_new(BULLET_SOURCE_ENEMY);
            local vx = 55 * arcdir[1];
            local vy = 55 * arcdir[2];
            bullet_set_position(nb, pos[1], pos[2]+35);
            bullet_set_visual(nb, PROJECTILE_SPRITE_WRM);
            bullet_set_scale(nb, 4, 4);
            bullet_set_visual_scale(nb, 0.5, 0.5);
            bullet_set_velocity(nb, vx, vy);
            t_wait(0.03);
        end
    end
end

function _Boss_Bombing_Run_0(e)
    TRAVEL_T = 2.6;
    if not enemy_valid(e) then return end
    enemy_set_velocity(e, 100, 0);
        async_task_lambda(
        function ()
            for i=0, 10 do
                if not enemy_valid(e) then return end
                laser_hazard_new(enemy_position_x(e), 10, 1, 0.05, 0.85, PROJECTILE_SPRITE_CAUSTIC);
                t_wait(TRAVEL_T/15);
            end
        end
        )
    t_wait(TRAVEL_T);
    -- sprout slow projectiles here
    async_task_lambda(_Boss_Sprout_0, e);

    if not enemy_valid(e) then return end
    enemy_set_velocity(e, 0, 0);
    t_wait(2.0);
    if not enemy_valid(e) then return end
    enemy_set_velocity(e, -100, 0);
        async_task_lambda(
        function ()
            for i=0, 10 do
                if not enemy_valid(e) then return end
                laser_hazard_new(enemy_position_x(e), 10, 1, 0.05, 0.85, PROJECTILE_SPRITE_CAUSTIC);
                t_wait(TRAVEL_T/15);
            end
        end
        )
    async_task_lambda(_Boss_Sprout_0, e);
    t_wait(TRAVEL_T);
    if not enemy_valid(e) then return end
    enemy_set_velocity(e, 0, 0);
    -- sprout slow projectiles here
end

function _Boss_Bombing_Run_1(e)
    TRAVEL_T = 0.8;
    if not enemy_valid(e) then return end
    enemy_set_velocity(e, 300, 0);
        async_task_lambda(
        function ()
            for i=0, 10 do
                if not enemy_valid(e) then return end
                laser_hazard_new(enemy_position_x(e), 10, 1, 0.05, 0.85, PROJECTILE_SPRITE_CAUSTIC);
                t_wait(TRAVEL_T/20);
            end
        end
        )
    t_wait(TRAVEL_T);
    -- sprout slow projectiles here
    async_task_lambda(_Boss_Sprout_1, e);

    if not enemy_valid(e) then return end
    enemy_set_velocity(e, 0, 0);
    t_wait(1.8);
    if not enemy_valid(e) then return end
    enemy_set_velocity(e, -300, 0);
        async_task_lambda(
        function ()
            for i=0, 10 do
                if not enemy_valid(e) then return end
                laser_hazard_new(enemy_position_x(e), 10, 1, 0.05, 0.85, PROJECTILE_SPRITE_CAUSTIC);
                t_wait(TRAVEL_T/20);
            end
        end
        )
    async_task_lambda(_Boss_Sprout_1, e);
    t_wait(TRAVEL_T);
    if not enemy_valid(e) then return end
    enemy_set_velocity(e, 0, 0);
    -- sprout slow projectiles here
end

function _Boss_Bombing_Run_2(e)
    TRAVEL_T = 0.8;
    if not enemy_valid(e) then return end
    enemy_set_velocity(e, 300, 0);
        async_task_lambda(
        function ()
            for i=0, 10 do
                if not enemy_valid(e) then return end
                laser_hazard_new(enemy_position_x(e), 10, 1, 0.05, 0.85, PROJECTILE_SPRITE_CAUSTIC);
                t_wait(TRAVEL_T/20);
            end
        end
        )
    t_wait(TRAVEL_T);
    async_task_lambda(_Boss_Sprout_0, e);
    async_task_lambda(_Boss_Sprout_1, e);

    if not enemy_valid(e) then return end
    enemy_set_velocity(e, 0, 0);
    t_wait(2.0);
    if not enemy_valid(e) then return end
    enemy_set_velocity(e, -300, 0);
        async_task_lambda(
        function ()
            for i=0, 10 do
                if not enemy_valid(e) then return end
                laser_hazard_new(enemy_position_x(e), 10, 1, 0.05, 0.85, PROJECTILE_SPRITE_CAUSTIC);
                t_wait(TRAVEL_T/20);
            end
        end
        )
    async_task_lambda(_Boss_Sprout_1, e);
    t_wait(TRAVEL_T);
    if not enemy_valid(e) then return end
    enemy_set_velocity(e, 0, 0);
    -- sprout slow projectiles here
end

function _Boss_Logic(e)
    while enemy_valid(e) do
        local hp_percent = enemy_hp_percent(e);
        if hp_percent <= 0.40 and subboss31_state.phase < 1 then
           subboss31_state.phase = 1;
        end

        if subboss31_state.phase == 0 then
            _Boss_Bombing_Run_0(e);
            t_wait(3.5);
            _Boss_Bombing_Run_0(e);
            t_wait(1.5);
            _Boss_Bombing_Run_1(e);
            t_wait(1.5);
            _Boss_Bombing_Run_1(e);
            t_wait(1.0);
            -- light little attacks sprouted after
        elseif subboss31_state.phase == 1 then
            _Boss_Bombing_Run_1(e);
            t_wait(1.6);
            _Boss_Bombing_Run_2(e);
            t_wait(1.5);
            _Boss_Bombing_Run_0(e);
            t_wait(1.6);
        end

        t_yield();
    end
end

function Game_Spawn_Stage3_1_SubBoss()
      local initial_boss_position = v2(play_area_width()/2 + 30, 95);

if enable_boss_intro then
      for i=0, 10 do
        explosion_hazard_new(0 + i * 20, 90, 25, 0.45-i*0.02, 0.55);
        explosion_hazard_new(play_area_width() - i * 20, 90, 25, 0.45-i*0.02, 0.55);
      end
      t_wait(0,25);
      for i=0, 10 do
        explosion_hazard_new(0 + i * 20, 130, 25, 0.45-i*0.02, 0.55);
        explosion_hazard_new(play_area_width() - i * 20, 130, 25, 0.45-i*0.02, 0.55);
      end
      t_wait(4);
end

      local e = enemy_new();
      enemy_begin_invincibility(e, true, 2.5);
      enemy_set_hp(e, SUBBOSS31_HP);
      enemy_set_position(e, initial_boss_position[1], initial_boss_position[2]);
	  enemy_set_visual(e, ENTITY_SPRITE_SUBBOSS31);
      enemy_set_scale(e, 40, 40);
      enemy_show_boss_hp(e, "GREATER SKULL");

      t_wait(0.7);
      enemy_set_acceleration(e, -120, 0);
      t_wait(1.58);
      enemy_set_acceleration(e, 0, 0);
      enemy_set_velocity(e, 0, 0);

      subboss31_state.me = e;
      subboss31_state.phase = 0;
      subboss31_state.last_good_position = enemy_final_position(e);
      async_task_lambda(_Boss_Logic, e);
      return e;
end