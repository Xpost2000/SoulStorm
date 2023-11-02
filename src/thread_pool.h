#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "common.h"

// #define SIMULATE_SYNCHRONOUS 1

typedef s32 (*job_queue_function)(void*);
// using job_queue_function = std::function<int(void*)>;
enum {
    THREAD_JOB_STATUS_FINISHED,
    THREAD_JOB_STATUS_READY,
    THREAD_JOB_STATUS_WORKING,
};

#define MAX_POSSIBLE_THREADS (16)
#define MAX_JOBS (512)

struct thread_job {
    u8                 status;
    void*              data;
    job_queue_function job;
};

namespace Thread_Pool {
    void add_job(job_queue_function job, void* data);
    void synchronize_tasks();
    void initialize();
    void synchronize_and_finish();
}

#endif
