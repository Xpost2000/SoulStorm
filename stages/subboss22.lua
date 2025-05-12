-- Handful of attack patterns but meant to be pretty short...

boss_state = {
    me,
    last_good_position, -- for if the entity is deleted for any reason
    starting_position,

    -- general boss logic state
    next_rain_attack_until = -9999,

    -- Attacking actions. Attack actions and movements
    -- are scheduled "simulatneously"
    next_think_action_t = -9999,
};

BOSS_HP = 1655;

function _Boss_Intro(e)
    enemy_begin_invincibility(e, true, 2.5);
    enemy_set_acceleration(e, 0, 120);
    t_wait(1.0);
    enemy_set_acceleration(e, 0, -120);
    t_wait(1.0);
    enemy_set_acceleration(e, 0, 0);
    enemy_set_velocity(e, 0, 0);
end

function SubBoss22_UnravelAttack1(epos, displacement, v)
    for i=0, 13 do
        local bspeed = i*20 + 70;
        for ang=90, 180, 15 do
            local arcdir = v2_direction_from_degree(ang-i + displacement);
            local b = bullet_make(
            BULLET_SOURCE_ENEMY,
            epos,
            v,
            v2(0.5, 0.5),
            v2(5, 5)
            )

            bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
        end
        for ang=0, 90, 15 do
            local arcdir = v2_direction_from_degree(ang+i + displacement);
            local b = bullet_make(
            BULLET_SOURCE_ENEMY,
            epos,
            v,
            v2(0.5, 0.5),
            v2(5, 5)
            )

            bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
        end
        t_wait(0.125);
    end
end

function SubBoss22_WhipAttack1(epos)
    local bspeed = 125;
    for i=0,7 do
        for ang=0, 360, 15 do
            local arcdir = v2_direction_from_degree(ang+i*10);
            local b = bullet_make(
            BULLET_SOURCE_ENEMY,
            v2(30, 40),
            PROJECTILE_SPRITE_HOT_PINK_ELECTRIC,
            v2(0.5, 0.5),
            v2(5, 5)
            )
            local arcdir1 = v2_direction_from_degree((-ang)-i*10);
            local b1 = bullet_make(
                BULLET_SOURCE_ENEMY,
                v2(play_area_width()-30, 40),
                PROJECTILE_SPRITE_HOT_PINK_ELECTRIC,
                v2(0.5, 0.5),
                v2(5, 5)
            )
    

            bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
            bullet_set_acceleration(b, arcdir[1] * bspeed/2, arcdir[2] * bspeed/2);

            bullet_set_velocity(b1, arcdir1[1] * bspeed, arcdir1[2] * bspeed);
            bullet_set_acceleration(b1, arcdir1[1] * bspeed/2, arcdir1[2] * bspeed/2);
            t_wait(0.05);
        end
    end
end

function SubBoss22_Attack0(epos)
    async_task_lambda(SubBoss22_UnravelAttack1, epos, 0, PROJECTILE_SPRITE_CAUSTIC);
    t_wait(3.5)
    async_task_lambda(SubBoss22_UnravelAttack1, epos, 25, PROJECTILE_SPRITE_RED);
    t_wait(1.5)
    async_task_lambda(SubBoss22_UnravelAttack1, epos, -15, PROJECTILE_SPRITE_CAUSTIC);
    t_wait(2.0)
end

function SubBoss22_ChaseDrop1(epos)
    async_task_lambda(
        function()
            local bspeed = 170;
            for i=0, 20 do
                local bpos = v2(20 * i, 30);
                local b = bullet_make(
                    BULLET_SOURCE_ENEMY,
                    bpos,
                    PROJECTILE_SPRITE_CAUSTIC,
                    v2(1, 1),
                    v2(10, 10)
                );
        
                local arcdir = dir_to_player(bpos);
                bullet_set_acceleration(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
                t_wait(0.25);
            end
        end
    )

    async_task_lambda(
        function()
            local bspeed = 160;
            for i=0, 20 do
                local bpos = v2(play_area_width() - 20 * i, 30);
                local b = bullet_make(
                    BULLET_SOURCE_ENEMY,
                    bpos,
                    PROJECTILE_SPRITE_CAUSTIC,
                    v2(1, 1),
                    v2(10, 10)
                );
        
                local arcdir = dir_to_player(bpos);
                bullet_set_acceleration(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
                t_wait(0.25);
            end
        end
    )
end

function SubBoss22_Attack1(epos)
    async_task_lambda(SubBoss22_WhipAttack1, epos);
    t_wait(0.5);
    async_task_lambda(SubBoss22_ChaseDrop1, epos);
    t_wait(3.5);
    async_task_lambda(SubBoss22_ChaseDrop1, epos);
end

function SubBoss22_VomitDirected(epos)
    for j=0,4 do
    for i=0,16 do
        for ang=-15, 15,(3+j) do
            local bspeed = 35+(i*14);
            local arcdir = dir_to_player(epos);
            local deg = math.atan(arcdir[2], arcdir[1]);
            local off = normalized_cos(deg);

            arcdir = v2_direction_from_degree(math.deg(deg) + ang);
            local b = bullet_make(
            BULLET_SOURCE_ENEMY,
            epos,
            PROJECTILE_SPRITE_HOT_PINK_ELECTRIC,
            v2(0.5, 0.5),
            v2(5, 5)
            )

            bullet_set_velocity(b, arcdir[1] * bspeed, arcdir[2] * bspeed);
            bullet_set_acceleration(b, arcdir[1] * bspeed/6, arcdir[2] * bspeed/6);
            t_wait(off*0.0136);
        end
    end
end
end

function _Boss_AttackPattern_Logic(e)
    local once=1;

    -- The boss doesn't really move or anything in this case. We're a static challenge.
    while enemy_valid(e) and (once==1) do
        local epos = enemy_final_position(e);
        SubBoss22_Attack0(epos);
        SubBoss22_Attack1(epos);
        t_wait(0.75);
        SubBoss22_Attack0(epos);
        SubBoss22_VomitDirected(epos);
        t_wait(0.88);
        SubBoss22_Attack0(epos);
        t_wait(1.2);
        SubBoss22_VomitDirected(epos);
        t_wait(0.75);
        SubBoss22_VomitDirected(epos);
        SubBoss22_Attack1(epos);
        -- SubBoss22_WhipAttack1(epos);
        once = 0;
        t_yield();
    end
end

function _Boss_Logic(e)
    _Boss_Intro(e);
    async_task_lambda(_Boss_AttackPattern_Logic, e);
end

function _Boss_Movement_Logic(e)
end

function Game_Spawn_Stage2_2_SubBoss()
    local e = enemy_new();
    local initial_boss_pos = v2(play_area_width()/2, -30);
    enemy_set_hp(e, BOSS_HP); -- TODO for now
    enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
    enemy_set_scale(e, 40, 64); -- chunky.
    -- enemy_set_visual(e, ENTITY_SPRITE_BOSS1);
    enemy_set_burst_gain_value(e, 0.325);
    enemy_show_boss_hp(e, "BONEWINGS");
    -- The boss takes "three threads" of logic.
    async_task_lambda(_Boss_Logic, e);
    -- async_task_lambda(_Boss_Movement_Logic, e);
    -- async_task_lambda(_Boss_ImmunityToBurstLaser_Logic, e);
    -- async_task_lambda(_Boss_MusicPlayer, e);
    boss_state.me = e;
    boss_state.starting_position = initial_boss_pos;
 
    return e;
 end