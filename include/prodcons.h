#ifndef _PRODCONS_H_
#define _PRODCONS_H_

#include <xinu.h>
/* Global variable for producer consumer */
extern int n; /* this is just declaration */

/* Semaphores */
extern sid32 can_read;
extern sid32 can_write;
extern sid32 done_prodcons;

/* Function Prototype */
void consumer(int count);
void producer(int count);

#endif //_PRODCONS_H_
