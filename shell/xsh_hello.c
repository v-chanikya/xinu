/* xsh_hello.c - xsh_hello */

#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <run_cmd.h>

/*---------------------------------------------------------------------------
 * xsh_hello - obtain user argument and appends a string hello in front of it
 *---------------------------------------------------------------------------
 */

shellcmd xsh_hello(int nargs, char *args[]) {
    int rv = 0;
    if(nargs ==  2){
        printf("Hello %s, Welcome to the world of Xinu!!\n", args[1]);
        rv = 0;
    }
    else{
        printf("Syntax: run hello name\n");
        rv =  1;
    }
    signal(run_complete);
    return rv;
}
