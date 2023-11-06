STAGE_DRAW(null) {
    return;
}

STAGE_TICK(null) {
    STAGE_TASK_DECLS;
    JDR_Coroutine_Start(co, Start);
    TASK_WAIT(1.5);
    TASK_COMPLETE_STAGE();
    JDR_Coroutine_End;
}

STAGE(null);
