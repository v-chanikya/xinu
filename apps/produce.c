#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>

void producer(int count) {
    for (int i=0; i <= count; i++){
        wait(can_write);
        n = i;
        printf("produced : %d\n", n);
        signal(can_read);
    }
    signal(done_prodcons);
}

void producer_bb(int id, int count) {
    for (int i=0; i < count; i++){
        wait(bb_can_write);
        arr_q_head = arr_q_head % (sizeof(arr_q)/sizeof(arr_q[0]));
        arr_q[arr_q_head] = i;
        arr_q_head++;
        printf("name : producer_%d, write : %d\n", id, i);
        signal(bb_can_read);
    }
    signal(done_prodcons_bb);
}
