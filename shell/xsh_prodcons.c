/* xsh_prodcons.c - xsh_date */

#include <xinu.h>
#include <stdlib.h>
#include <prodcons.h>

/*------------------------------------------------------------------------
 * xsh_prodcons - producer consumer problem
 *------------------------------------------------------------------------
 */
int n;

shellcmd xsh_prodcons(int nargs, char *args[]) {
    int count = 2000;
    if (nargs > 2)
        return -1;
    
    if (nargs == 2)
        if ((count = atoi(args[1])) < 0)
            return -1;

    resume(create(producer, 1024, 20, "producer", 1, count));
    resume(create(consumer, 1024, 20, "consumer", 1, count));
    return 0;
}
