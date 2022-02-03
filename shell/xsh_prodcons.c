/* xsh_prodcons.c - xsh_date */

#include <xinu.h>
#include <stdlib.h>
#include <prodcons.h>
#include <run_cmd.h>

/*------------------------------------------------------------------------
 * xsh_prodcons - producer consumer problem
 *------------------------------------------------------------------------
 */
int n;
sid32 can_read;
sid32 can_write;
sid32 done_prodcons;

shellcmd xsh_prodcons(int nargs, char *args[]) {
    int count = 200;
    if (nargs > 2){
        printf("Syntax: run prodcons [counter]\n");
        goto fail;
    }
    
    if (nargs == 2){
        if ((count = atoi(args[1])) < 0){
            printf("Syntax: run prodcons [counter]\n");
            goto fail;
        }
    }

    /* Create semaphores */
    can_read = semcreate(0);
    can_write = semcreate(1);
    done_prodcons = semcreate(0);

    resume(create(producer, 1024, 20, "producer", 1, count));
    resume(create(consumer, 1024, 20, "consumer", 1, count));

    wait(done_prodcons);
    wait(done_prodcons);
    signal(run_complete);
    return 0;

fail:
    signal(run_complete);
    return -1;
}
