// Jerry's Duff Routines, a small SFL for myself...
// This is the traditional API version, which mimics real coroutine libraries
// even though this duff routine has the advantage of not needing magical pointers.
#ifndef DUFF_COROUTINE_H
#define DUFF_COROUTINE_H

/*
 * At the moment setjmp and longjmp frankly
 * concern me for many multiple reasons.
 *
 * Lots of portability problems, and I don't really
 * want to use a coroutine library just to do this,
 * since the rest of this project was deliberately simple
 * but allowed me to go technical on certain aspects.
 *
 * I'm not exactly about to back off...
 *
 * NOTE: since this is a duff's device based 'resumable function'
 * it doesn't work the same way as other coroutine apis.
 */

/*
 * Anyways this is a simple "Resumable Function" with
 * Duff's Device.
 *
 * This isn't really a coroutine but for the reasons I use coroutines
 * it's good enough.
 *
 *
 * NOTE: This is generally going to assume you do the sane thing of allocating your
 *       memory separately
 *
 * Admittedly this is more flexible than traditional C coroutine APIs
 * because I don't need a function pointer, and in reality I'm kind of just
 * hijacking a normal function.
 *
 * However, I should provide a more "traditional" API in case I use a more "technically" correct
 * implementation. Like with fibers or something.
 */

#define JDR_COROUTINE_STACK_MAX_SIZE (1) // NOTE for SoulStorm: I store state manually. So it's not needed...
#define JDR_COROUTINE_MAX_DEPTH      (1)
#define JDR_ARRAYCOUNT(x)            sizeof(x)/sizeof(*x)

#ifndef JDR_ASSERT
#include "assert.h"
#define JDR_ASSERT(x) assert(x)
#endif


struct jdr_duffcoroutine;
typedef void (*jdr_duffcoroutine_fn)(struct jdr_duffcoroutine*);
typedef struct jdr_duffcoroutine {
    // NOTE: shared stack
    char  stack[JDR_COROUTINE_STACK_MAX_SIZE];
    int   stackused;
    int   state[JDR_COROUTINE_MAX_DEPTH];
    int   depth;
    int   status;

    /*
      NOTE:
      the userdata ptr is both used as input/output.

      It is input from a resume/awaken

      as well as output from a yield
    */
    void* userdata;
    jdr_duffcoroutine_fn f;
} jdr_duffcoroutine_t;

enum jdr_duffcoroutine_status {
    JDR_DUFFCOROUTINE_SUSPENDED = 0,
    JDR_DUFFCOROUTINE_RUNNING   = 1,
    JDR_DUFFCOROUTINE_FINISHED  = 2,
};

jdr_duffcoroutine_t jdr_coroutine_new(jdr_duffcoroutine_fn f);
void                jdr_coroutine_rewind(jdr_duffcoroutine_t* co);

// NOTE: status code is mostly for yourself to check.
int                 jdr_coroutine_status(jdr_duffcoroutine_t* co);
int                 jdr_resume(jdr_duffcoroutine_t* co);

jdr_duffcoroutine_t* _jdr_current();
void*               _jdr_alloc_var(int sz);
void                _jdr_bind_current(jdr_duffcoroutine_t* co);
/* #define jdr_val(T, v) &(*((T*)_jdr_alloc_var(sizeof(T))) = v) */

#define _JDR_TOKEN_CONCAT(a, b) a ## b
#define JDR_Coroutine_Start(co, lbl)  _jdr_bind_current(co); switch(_jdr_current()->state[_jdr_current()->depth++]) { case 0: _jdr_current()->status = JDR_DUFFCOROUTINE_RUNNING; lbl: (void)"terminatewithsemicolon"
#define _JDR_Cleanup                  do { _jdr_current()->depth--; _jdr_current()->stackused = 0; if(_jdr_current()->depth == 0) _jdr_current()->status = JDR_DUFFCOROUTINE_FINISHED; } while (0)
#define _JDR_YIELD_BODY _jdr_current()->state[_jdr_current()->depth-1] = __LINE__; _JDR_Cleanup; _jdr_current()->status = JDR_DUFFCOROUTINE_SUSPENDED;
#define JDR_Coroutine_Yield(x)        do {_JDR_YIELD_BODY; _jdr_current()->userdata = x; return; case __LINE__:{}} while(0);
#define JDR_Coroutine_YieldNR()       do {_JDR_YIELD_BODY; return; case __LINE__:{}} while(0);
#define JDR_Coroutine_End             default: _JDR_Cleanup; }

#ifdef JDR_COROUTINE_IMPLEMENTATION
jdr_duffcoroutine_t* _current_cr_obj = 0;

void jdr_coroutine_rewind(jdr_duffcoroutine_t* co) {
    int i;

    for (i = 0; i < JDR_ARRAYCOUNT(co->stack); ++i) co->stack[i] = 0;
    for (i = 0; i < JDR_ARRAYCOUNT(co->state); ++i) co->state[i] = 0;

    co->status    = JDR_DUFFCOROUTINE_SUSPENDED;
    co->stackused = 0;
    co->depth     = 0;
}

int jdr_resume(jdr_duffcoroutine_t* co) {
    if (co->status != JDR_DUFFCOROUTINE_FINISHED) {
        co->f(co);
        return 1;
    }

    return 0;
}

jdr_duffcoroutine_t jdr_coroutine_new(jdr_duffcoroutine_fn f) {
    jdr_duffcoroutine_t co;
    jdr_coroutine_rewind(&co);
    co.f = f;
    return co;
}

jdr_duffcoroutine_t* _jdr_current() {
    return _current_cr_obj;
}

void _jdr_bind_current(jdr_duffcoroutine_t* co) {
    _current_cr_obj = co;
}

int jdr_coroutine_status(jdr_duffcoroutine_t* co) {
    return co->status;
}

void* _jdr_alloc_var(int sz) {
    JDR_ASSERT(_current_cr_obj && "There should be a bound Coroutine... Did you forget to JDR_Coroutine_Start ?");
    void* ptr;
    ptr = (void*)(_current_cr_obj->stack + _current_cr_obj->stackused);
    _current_cr_obj->stackused += sz;
    return ptr;
}
#endif

#endif
