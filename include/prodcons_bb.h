#ifndef _PRODCONS_BB_H_
#define _PRODCONS_BB_H_

#include <xinu.h>
// declare globally shared array
extern int arr_q[5];

// declare globally shared semaphores
/* Semaphores */
extern sid32 bb_can_read;
extern sid32 bb_can_write;
extern sid32 done_prodcons_bb;

// declare globally shared read and write indices
extern int arr_q_head;
extern int arr_q_tail;

// function prototypes
extern int prodcons_bb(int nargs, char *args[]);
extern void consumer_bb(int id, int count);
extern void producer_bb(int id, int count);

#endif //_PRODCONS_BB_H_
