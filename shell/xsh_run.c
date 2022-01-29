/* xsh_run.c - xsh_run */

#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <shprototypes.h>

/*----------------------------------------------------------------------------------
 * xsh_run - Runs the user provided command if it is in the list of allowed commands
 *----------------------------------------------------------------------------------
 */

typedef struct vcmds{
    char *cmd;
    void *func;
}vcmds;

void list_cmds(vcmds *cmds, int ncmds){
    vcmds *iter = cmds;
    for (int i=0; i < ncmds; i++){
        printf("%s\n", iter->cmd);
        iter++;
    }
}

shellcmd xsh_run(int nargs, char *args[]) {

    vcmds allowed_cmds[] = {
        {"list", (void*) list_cmds},
        {"hello", (void*) xsh_hello},
        {"prodcons", (void*) xsh_prodcons}
    };

    int ncmds = sizeof(allowed_cmds)/sizeof(vcmds);

    if(nargs ==  1){
        /* fprintf(stderr, "Syntax: run [cmd] [args]\n"); */
        /* fprintf(stderr, "List of supported commands\n"); */
        list_cmds(allowed_cmds, ncmds);
        return 1;
    }
    else{
        if (strncmp(args[1], "list", 4) == 0){
            list_cmds(allowed_cmds, ncmds);
            return 0;
        }
        nargs--;
        args++;
        
        for (int i=1; i < ncmds; i++){
            if (strncmp(args[0], allowed_cmds[i].cmd, strlen(allowed_cmds[i].cmd)) == 0){
                resume (create(allowed_cmds[i].func, 4096, 20, allowed_cmds[i].cmd, 2, nargs, args));
                return 0;
            }
        }

        list_cmds(allowed_cmds, ncmds);
        return 1;
    }
}
