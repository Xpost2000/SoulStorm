DEF_LEVEL_DATA(1_1) {

};

STAGE_DRAW(1_1) {
    LEVEL_DATA(1_1, data);

    return;
}

/*
  Fortunately this is basically a tutorial stage, so there's not much problem with just
  having entities that don't shoot for a bit before introducing it.
*/
STAGE_TICK(1_1) {
    STAGE_TASK_DECLS;
    LEVEL_DATA(1_1, data);

    JDR_Coroutine_Start(co, Start);
    TASK_WAIT(0.5);

    {
        for (int i = 0; i < 3; ++i)  {
            auto e = enemy_linear_movement(state, V2(i * 35, -30), V2(10, 10), V2(0, 1), 125);
            e.hp = 10; state->gameplay_data.add_enemy_entity(e);
        }

        for (int i = 0; i < 3; ++i)  {
            auto e = enemy_linear_movement(state, V2(gameplay_state->play_area.width - i * 35, -30), V2(10, 10), V2(0, 1), 125);
            e.hp = 10; state->gameplay_data.add_enemy_entity(e);
        }
    }
    TASK_WAIT(1.0);
    {
        for (int i = 0; i < 10; ++i)  {
            auto e = enemy_linear_movement(state, V2(i * 35, -30), V2(10, 10), V2(0, 1), 155);
            e.hp = 10; state->gameplay_data.add_enemy_entity(e);
        }
    }
    TASK_WAIT(1.0);
    {
        auto e = enemy_linear_movement(state, V2(0 * 50, -40), V2(10, 10), V2(0, 1), 250);
        e.hp = 10; state->gameplay_data.add_enemy_entity(e);
    }
    TASK_WAIT(0.25);
    {
        auto e = enemy_linear_movement(state, V2(1 * 50, -40), V2(10, 10), V2(0, 1), 250);
        e.hp = 10; state->gameplay_data.add_enemy_entity(e);
    }
    TASK_WAIT(0.25);

    // Entity 17
    {
        auto e = enemy_linear_movement(state, V2(2 * 50, -40), V2(10, 10), V2(0, 1), 250);
        e.hp = 10; state->gameplay_data.add_enemy_entity(e);
    }

    TASK_WAIT(0.25);
    WITH_ENTITY(gameplay_state->lookup_enemy(17)) {
        it->reset_movement();
        state->coroutine_tasks.add_task(
            state,
            [](struct jdr_duffcoroutine* co) {
                STAGE_TASK_DECLS;
                auto e = gameplay_state->lookup_enemy(17);
                JDR_Coroutine_Start(co, Start);
                while (e) {
                    spawn_bullet_arc_pattern2(state, e->position, 5, 45, V2(5, 5), V2(0, 1), 1000, 0, BULLET_SOURCE_ENEMY);
                    TASK_WAIT(0.15);
                }
                JDR_Coroutine_End;
            }
        );
    }
    {
        auto e = enemy_linear_movement(state, V2(3 * 50, -40), V2(10, 10), V2(0, 1), 250);
        e.hp = 10; state->gameplay_data.add_enemy_entity(e);
    };
    TASK_WAIT(0.25);
    {
        auto e = enemy_linear_movement(state, V2(4 * 50, -40), V2(10, 10), V2(0, 1), 250);
        e.hp = 10; state->gameplay_data.add_enemy_entity(e);
    };
    TASK_WAIT(0.25);
    {
        auto e = enemy_linear_movement(state, V2(5 * 50, -40), V2(10, 10), V2(0, 1), 250);
        e.hp = 10; state->gameplay_data.add_enemy_entity(e);
    };
    TASK_WAIT(0.25);
    STAGE_WAIT_CLEARED_WAVE();

    TASK_WAIT(1.5);
    TASK_COMPLETE_STAGE();
    JDR_Coroutine_End;
} 

STAGE(1_1);
