#include <xinu.h>
#include <fs_driver.h>
#include <run_cmd.h>

void fstest_driver(int nargs, char *args[]){
    fstest(nargs, args);
    signal(run_complete);
}
