DEF_LEVEL_DATA(null) {

};

STAGE_DRAW(null) {
    LEVEL_DATA(null, data);
    return;
}

STAGE_TICK(null) {
    STAGE_TASK_DECLS;
    LEVEL_DATA(null, data);
    JDR_Coroutine_Start(co, Start);
    TASK_WAIT(1.5);
    TASK_COMPLETE_STAGE();
    JDR_Coroutine_End;
}

STAGE(null);
