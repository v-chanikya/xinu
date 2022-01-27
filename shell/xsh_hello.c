/* xsh_hello.c - xsh_hello */

#include <xinu.h>
#include <string.h>
#include <stdio.h>

/*---------------------------------------------------------------------------
 * xsh_hello - obtain user argument and appends a string hello in front of it
 *---------------------------------------------------------------------------
 */
shellcmd xsh_hello(int nargs, char *args[]) {
    if(nargs ==  2){
        printf("Hello %s, Welcome to the world of Xinu!!\n", args[1]);
        return 0;
    }
    else{
        fprintf(stderr, "Error expected a single input\n");
        return 1;
    }
}
