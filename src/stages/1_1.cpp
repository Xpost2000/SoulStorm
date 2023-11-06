STAGE_DRAW(1_1) {
    return;
}

STAGE_TICK(1_1) {
    STAGE_TASK_DECLS;
    JDR_Coroutine_Start(co, Start);
    for (int i = 0; i < 6; ++i) 
        spawn_enemy_linear_movement(state, V2(i * 35, -30), V2(10, 10), V2(0, 1), 150);

    while (gameplay_state->any_living_danger()) {
        TASK_YIELD();
    }

    TASK_WAIT(1.5);
    TASK_COMPLETE_STAGE();
    JDR_Coroutine_End;
} 

STAGE(1_1);
