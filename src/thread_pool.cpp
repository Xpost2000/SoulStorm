#include <SDL2/SDL.h>
#include "thread_pool.h"
#include "memory_arena.h"

#ifdef __EMSCRIPTEN__
// The workers in emscripten I do not trust.
#define SIMULATE_SYNCHRONOUS
#endif

#include "engine.h"

struct thread_job_queue {
    struct thread_job jobs[MAX_JOBS];
    SDL_sem*          notification;
    SDL_mutex*        mutex;
};

local volatile s32      global_thread_count                             = 0;
local SDL_Thread*       global_thread_pool[MAX_POSSIBLE_THREADS]        = {};
local Memory_Arena      global_thread_pool_arenas[MAX_POSSIBLE_THREADS] = {};
struct thread_job_queue global_job_queue                                = {};

namespace Thread_Pool {
    void add_job(job_queue_function job, void* data) {
#ifdef SIMULATE_SYNCHRONOUS
        job(data);
#else
        for (s32 index = 0; index < MAX_JOBS; ++index) {
            struct thread_job* current_job = &global_job_queue.jobs[index];
        
            if (current_job->status == THREAD_JOB_STATUS_FINISHED) {
                current_job->status = THREAD_JOB_STATUS_READY;
                current_job->data   = data;
                current_job->job    = job;
#if 0
                _debugprintf("posted new job (%p dataptr)", current_job->data);
#endif

                SDL_SemPost(global_job_queue.notification);
                return;
            }
        }
        #endif
    }

    void synchronize_tasks() {
#ifdef SIMULATE_SYNCHRONOUS
        return; // busy body.
#else
        bool done = false;
        /* NOTE:
           fix this, I'm pretty sure this is the cause of those mysterious,
           on exit crashes, since I suspect sometimes the thread jobs never get
           the chance to synchronize since the game stops running and will stop syncing.
        */
        while (!done && Global_Engine()->running) {
            done = true;

            for (s32 index = 0; index < MAX_JOBS; ++index) {
                struct thread_job* current_job = global_job_queue.jobs + index;
                if (current_job->status != THREAD_JOB_STATUS_FINISHED) {
                    done = false;
                }
            }
        }
#endif
    }

    static int _thread_job_executor(void* context) {
        Memory_Arena* arena   = (Memory_Arena*)context;
        Memory_Arena  scratch = arena->sub_arena(Kilobyte(64));
        // I'll note that I haven't had to actually use the memory yet,
        // but at least it's here if I need it...
#if 1
        _debugprintf("thread start : \"%d\"", (s32)SDL_ThreadID());
#endif

        struct thread_job* working_job = 0;

        for (;;) {
            if (!SDL_SemWait(global_job_queue.notification)) {
                if (!Global_Engine()->running) {
                    break;
                }

                /* job hunting! */
                /* let's hunt */
                SDL_LockMutex(global_job_queue.mutex);
                {
                    while (!working_job && Global_Engine()->running) {
                        for (s32 index = 0; index < MAX_JOBS && Global_Engine()->running; ++index) {
                            struct thread_job* current_job = &global_job_queue.jobs[index];

                            if (current_job->status == THREAD_JOB_STATUS_READY) {
                                current_job->status = THREAD_JOB_STATUS_WORKING;
                                working_job = current_job;
                                break;
                            }
                        }
                    }
                }
                SDL_UnlockMutex(global_job_queue.mutex);
            }

            if (working_job && working_job->job) {
                working_job->job(working_job->data);
                working_job->status = THREAD_JOB_STATUS_FINISHED;
                working_job         = NULL;
            }
        }

        _debugprintf("Thread (%d) quits", (s32)SDL_ThreadID());
        return 0;
    }

    void initialize(void) {
#ifdef SIMULATE_SYNCHRONOUS
        _debugprintf("Thread system is in synchronous mode for profiling and predictability.");
        return;
#else
        s32 cpu_count = SDL_GetCPUCount();
        global_thread_count = cpu_count;
        _debugprintf("%d cpus reported.", cpu_count);

        global_job_queue.notification = SDL_CreateSemaphore(0);
        global_job_queue.mutex        = SDL_CreateMutex();

        if (global_thread_count > MAX_POSSIBLE_THREADS) global_thread_count = MAX_POSSIBLE_THREADS;
        for (s32 index = 0; index < global_thread_count; ++index) {
            _debugprintf("Trying to make thread %d", index);
            global_thread_pool_arenas[index] = Memory_Arena((char*)"thread pool", Kilobyte(256));
            global_thread_pool[index]        = SDL_CreateThread(_thread_job_executor, format_temp("slave%d", index), &global_thread_pool_arenas[index]);
        }
#endif
    }

    s32 active_jobs() {
        s32 count = 0;

        for (s32 index = 0; index < MAX_JOBS; ++index) {
            struct thread_job* current_job = global_job_queue.jobs + index;
            if (current_job->status != THREAD_JOB_STATUS_FINISHED) {
                count += 1;
            }
        }

        return count;
    }

    void synchronize_and_finish(void) {
#ifdef SIMULATE_SYNCHRONOUS
        _debugprintf("lol");
        return;
#else
        _debugprintf("Trying to synchronize and finish the threads... Please");
        /* signal to quit the threads since we do a blocking wait */

        _debugprintf("semaphore value: %d (have to kill %d threads)", SDL_SemValue(global_job_queue.notification), global_thread_count);
        /* 
           notify the semaphore up to the amount of threads,
       
           semaphore is accessed, and should atomically decrement and the thread should quit.

           I have no idea why I have to separate the waiting honestly. Seems to deadlock in that
           case.
       
           Considering I only have one mutex and one semaphore I'm surprised I can cause a deadlock with
           a situation that simple...
        */
        for (s32 thread_index = 0; thread_index < global_thread_count; ++thread_index) {
            _debugprintf("posting semaphore, and hoping a thread dies");
            SDL_SemPost(global_job_queue.notification);
        }
        synchronize_tasks();

        for (s32 thread_index = 0; thread_index < global_thread_count; ++thread_index) {
            _debugprintf("waiting on thread");
            SDL_WaitThread(global_thread_pool[thread_index], NULL);
            _debugprintf("thread should be dead");
        }

        for (s32 thread_index = 0; thread_index < global_thread_count; ++thread_index) {
            global_thread_pool_arenas[thread_index].finish();
        }

        SDL_DestroySemaphore(global_job_queue.notification);
        SDL_DestroyMutex(global_job_queue.mutex);
#endif
    }
}
