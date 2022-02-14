#include <future.h>

future_t* future_alloc(future_mode_t mode, uint size, uint nelem) {
    intmask mask;
    mask = disable();
    
    future_t *fut  =  (future_t *) getmem(sizeof(future_t));
    if (!fut)
        return (future_t*)SYSERR;

    fut->mode   = mode;
    fut->size   = size;
    fut->state  = FUTURE_EMPTY;

    restore(mask);
    return fut;
}

syscall future_free(future_t* fut) {
    intmask mask;
    mask = disable();

    int fail = 0;
    if (fut->data)
        if (freemem(fut->data, fut->size) != OK)
            fail = 1;

    if (freemem((char *) fut, sizeof(future_t)) != OK)
        fail = 1;

    // TD Kill process
    if (fut->pid)
        kill(fut->pid);

    restore(mask);
    if (fail)
        return SYSERR;
    else
        return OK;
}

syscall future_get(future_t* fut, char** out) {
    intmask mask;
    mask = disable();

    int fail = 0;

    switch (fut->state){
        case FUTURE_EMPTY:
            fut->state = FUTURE_WAITING;
            fut->pid = getpid();
            break;
        case FUTURE_READY:
            *out = fut->data;
            fut->data = NULL;
            fut->state = FUTURE_EMPTY;
            break;
        case FUTURE_WAITING:
            fail = 1;
            break;
    }

    restore(mask);
    if (fail)
        return SYSERR;
    else
        return OK;
}

syscall future_set(future_t* fut, char* in){
    intmask mask;
    mask = disable();

    int fail = 0;

    switch (fut->state){
        case FUTURE_WAITING:
            fut->pid = 0;
        case FUTURE_EMPTY:
            fut->data = in;
            fut->state = FUTURE_READY;
            break;
        case FUTURE_READY:
            fail = 1;
            break;
    }

    restore(mask);
    if (fail)
        return SYSERR;
    else
        return OK;
}
