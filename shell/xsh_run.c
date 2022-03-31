/* xsh_run.c - xsh_run */

#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <shprototypes.h>
#include <run_cmd.h>
#include <prodcons_bb.h>
#include <future_prodcons.h>
#include <streams.h>

/*----------------------------------------------------------------------------------
 * xsh_run - Runs the user provided command if it is in the list of allowed commands
 *----------------------------------------------------------------------------------
 */

sid32 run_complete;

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
        {"futest", (void*) future_prodcons},
        {"hello", (void*) xsh_hello},
        {"list", (void*) list_cmds},
        {"prodcons", (void*) xsh_prodcons},
        {"prodcons_bb", (void*) prodcons_bb},
        {"tscdf", (void*) stream_proc},
        {"tscdf_fq", (void*) stream_proc_futures},
    };

    int ncmds = sizeof(allowed_cmds)/sizeof(vcmds);

    if(nargs ==  1){
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
        
        int arg_len = strlen(args[0]);
        for (int i=0; i < ncmds; i++){
            if (arg_len == strlen(allowed_cmds[i].cmd) && strncmp(args[0], allowed_cmds[i].cmd, arg_len) == 0){
                run_complete = semcreate(0);
                resume (create(allowed_cmds[i].func, 4096, 20, allowed_cmds[i].cmd, 2, nargs, args));
                wait(run_complete);
                return 0;
            }
        }

        list_cmds(allowed_cmds, ncmds);
        return 1;
    }
}
