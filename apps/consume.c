#include <xinu.h>
#include <prodcons.h>

extern int n;

void consumer(int count) {
    for (int i=0; i <= count; i++){
        wait(can_read);
        printf("consumed : %d\n", n);
        signal(can_write);
    }
}
