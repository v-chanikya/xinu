#include <prodcons_bb.h>
#include <run_cmd.h>
#include <stdlib.h>

// definition of array, semaphores and indices

sid32 bb_can_read;
sid32 bb_can_write;
sid32 done_prodcons_bb;
int arr_q[5];
int arr_q_head;
int arr_q_tail;

int prodcons_bb(int nargs, char *args[]) {

    int prod_n = 0;
    int cons_n = 0;
    int prod_r = 0;
    int cons_r = 0;

    if (nargs != 5){
        printf("Syntax: run prodcons_bb <# of producer processes> <# of consumer processes> <# of iterations the producer runs> <# of iterations the consumer runs>\n");
        goto fail;
    }else{
        prod_n =  atoi(args[1]);
        cons_n =  atoi(args[2]);
        prod_r =  atoi(args[3]);
        cons_r =  atoi(args[4]);
    }

    if (prod_n*prod_r == 0 || cons_n*cons_r == 0 ||
            prod_n*prod_r != cons_n*cons_r){
        printf("Iteration Mismatch Error: the number of producer(s) iteration does not match the consumer(s) iteration\n");
        goto fail;
    }
    /* Create semaphores */
    bb_can_read = semcreate(0);
    bb_can_write = semcreate(1);
    done_prodcons_bb = semcreate(0);

    //initialize read and write indices for the queue
    arr_q_head = 0;
    arr_q_tail = 0;

    //create producer and consumer processes and put them in ready queue
    for (int iter = 0; iter < prod_n; iter++)
        resume(create(producer_bb, 1024, 20, "producer", 2, iter, prod_r));
    for (int iter = 0; iter < cons_n; iter++)
        resume(create(consumer_bb, 1024, 20, "consumer", 2, iter, cons_r));

    for (int iter = 0; iter < prod_n + cons_n; iter++)
        wait(done_prodcons_bb);

    signal(run_complete);
    return 0;

fail:
    signal(run_complete);
    return -1;
}
