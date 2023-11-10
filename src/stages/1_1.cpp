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
    TASK_WAIT(1.5);
    {
        for (int i = 0; i < 10; ++i)  {
            auto e = enemy_linear_movement(state, V2(i * 35, -30), V2(10, 10), V2(0, 1), 155);
            e.hp = 10; state->gameplay_data.add_enemy_entity(e);
        }
    }
    STAGE_WAIT_CLEARED_WAVE();

    TASK_WAIT(1.5);
    TASK_COMPLETE_STAGE();
    JDR_Coroutine_End;
} 

STAGE(1_1);
