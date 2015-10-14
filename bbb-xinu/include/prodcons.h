#include <future.h>
#include <stdio.h>

/*Global variable for producer consumer*/
extern int n; //global declaration

/* Declare the required semaphores */
extern sid32 consumed, produced;   

/*function Prototype*/
void producer(sid32, sid32, int);
void consumer(sid32, sid32, int);
