#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>

void consumer(int count) {
    for (int i=0; i <= count; i++){
        wait(can_read);
        printf("consumed : %d\n", n);
        signal(can_write);
    }
    signal(done_prodcons);
}

void consumer_bb(int id, int count) {
    for (int i=0; i < count; i++){
        wait(bb_can_read);
        arr_q_tail = arr_q_tail % (sizeof(arr_q)/sizeof(arr_q[0]));
        printf("name : consumer_%d, read : %d\n", id, arr_q[arr_q_tail]);
        arr_q_tail++;
        signal(bb_can_write);
    }
    signal(done_prodcons_bb);
}
