#include <future.h>

future_t* future_alloc(future_mode_t mode, uint size, uint nelem) {
    intmask mask;
    mask = disable();
    
    future_t *fut  =  (future_t *) getmem(sizeof(future_t));
    if (!fut){
        restore(mask);
        return (future_t *)SYSERR;
    }

    fut->mode       = mode;
    fut->size       = size;
    fut->state      = FUTURE_EMPTY;
    fut->max_elems  = nelem;
    fut->head       = 0;
    fut->tail       = 0;
    fut->count      = 0;

    if(size * nelem)
        fut->data = getmem(size * nelem);
    else
        fut->data = NULL;

    switch (mode) {
        case FUTURE_QUEUE:
            if ((fut->set_queue = newqueue()) == SYSERR){
                restore(mask);
                return (future_t *)SYSERR;
            }
        case FUTURE_SHARED:
            if ((fut->get_queue = newqueue()) == SYSERR){
                restore(mask);
                return (future_t *)SYSERR;
            }
            break;
    }

    restore(mask);
    return fut;
}

syscall future_free(future_t* fut) {
    pid32 pid;
    intmask mask;
    mask = disable();

    int fail = 0;
    // Kill all pending processes
    switch (fut->mode){
        case FUTURE_QUEUE:
            while ((pid = dequeue(fut->set_queue)) != EMPTY)
                kill(pid);
            delqueue(fut->get_queue);
            fut->get_queue = -1;
        case FUTURE_SHARED:
            while ((pid = dequeue(fut->get_queue)) != EMPTY)
                kill(pid);
            delqueue(fut->get_queue);
            fut->get_queue = -1;
            break;
        case FUTURE_EXCLUSIVE:
            if (fut->pid){
                kill(fut->pid);
                fut->pid = -1;
            }
            break;
    }

    if (fut->data != NULL){
        if (freemem(fut->data, fut->size * fut->max_elems) != OK){
            fail = 1;
        }
        fut->data = NULL;
    }

    if (freemem((char *) fut, sizeof(future_t)) != OK){
        fail = 1;
    }

    restore(mask);
    if (fail)
        return SYSERR;
    else
        return OK;
}

syscall future_get(future_t* fut, char* out) {
    struct procent *prptr;
    intmask mask;
    mask = disable();

    int fail = 0;
    pid32 pid;

    switch (fut->mode){
        case FUTURE_EXCLUSIVE:
            switch (fut->state){
                case FUTURE_EMPTY:
                    prptr           = &proctab[currpid];
                    prptr->prstate  = PR_FUWAIT;
                    fut->state      = FUTURE_WAITING;
                    fut->pid        = currpid;
                    resched();
                    *out = *fut->data;
                    fut->data = NULL;
                    fut->state = FUTURE_EMPTY;
                    break;
                case FUTURE_READY:
                    *out = *fut->data;
                    fut->data = NULL;
                    fut->state = FUTURE_EMPTY;
                    break;
                case FUTURE_WAITING:
                    fail = 1;
                    break;
            }
            break;
        case FUTURE_SHARED:
            switch (fut->state){
                case FUTURE_READY:
                    for(int i = 0; i < fut->size; i++){
                        *(out + i) = *(fut->data + i);
                    }
                    break;
                case FUTURE_EMPTY:
                case FUTURE_WAITING:
                    prptr           = &proctab[currpid];
                    prptr->prstate  = PR_FUWAIT;
                    fut->state      = FUTURE_WAITING;
                    if ((enqueue(currpid, fut->get_queue)) == SYSERR){
                        fail = 1;
                        break;
                    }
                    resched();
                    for(int i = 0; i < fut->size; i++){
                        *(out + i) = *(fut->data + i);
                    }
                    break;
            }
            break;
        case FUTURE_QUEUE:
            if (fut->count == 0){
                if ((enqueue(currpid, fut->get_queue)) == SYSERR){
                    fail = 1;
                    /* break; */
                }
                resched();
            }
            for(int i = 0; i < fut->size; i++){
                *(out + i) = *(fut->data + (fut->head * fut->size) + i);
            }
            fut->head = (fut->head + 1) % fut->max_elems;
            fut->count -= 1;
            if (fut->count == fut->max_elems - 1)
                if ((pid = dequeue(fut->set_queue)) != EMPTY)
                    ready(pid);
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
    pid32 pid;

    int fail = 0;

    switch (fut->mode){
        case FUTURE_EXCLUSIVE:
            switch (fut->state){
                case FUTURE_WAITING:
                    fut->data = in;
                    fut->state = FUTURE_READY;
                    ready(fut->pid);
                    fut->pid = 0;
                    break;
                case FUTURE_EMPTY:
                    fut->data = in;
                    fut->state = FUTURE_READY;
                    break;
                case FUTURE_READY:
                    fail = 1;
                    break;
            }
            break;
        case FUTURE_SHARED:
            switch (fut->state){
                case FUTURE_EMPTY:
                    for(int i = 0; i < fut->size; i++){
                        *(fut->data + i) = *(in + i);
                    }
                    fut->state = FUTURE_READY;
                    break;
                case FUTURE_READY:
                    fail = 1;
                    break;
                case FUTURE_WAITING:
                    for(int i = 0; i < fut->size; i++){
                        *(fut->data + i) = *(in + i);
                    }
                    fut->state = FUTURE_READY;
                    while ((pid = dequeue(fut->get_queue)) != EMPTY){
                        ready(pid);
                    }
                    break;
            }
            break;
        case FUTURE_QUEUE:
            if (fut->count == fut->max_elems){
                if ((enqueue(currpid, fut->set_queue)) == SYSERR){
                    fail = 1;
                    break;
                }
                resched();
            }
            for(int i = 0; i < fut->size; i++){
                *(fut->data + (fut->tail * fut->size) + i) = *(in + i);
            }
            fut->tail = (fut->tail + 1) % fut->max_elems;
            fut->count += 1;
            if (fut->count == 1)
                if ((pid = dequeue(fut->get_queue)) != EMPTY)
                    ready(pid);
            break;
    }

    restore(mask);
    if (fail)
        return SYSERR;
    else
        return OK;
}
