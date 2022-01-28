#include <xinu.h>
#include <prodcons.h>

extern int n;

void producer(int count) {
    for (int i=0; i <= count; i++){
        wait(can_write);
        n = i;
        printf("produced : %d\n", n);
        signal(can_read);
    }
}
