#include <prodcons.h>

void producer(sid32 produced, sid32 consumed, int count)
{
	int i=1;

	//Use system call wait() and signal() with predefined semaphores produced and consumed to synchronize critical section
	//Code to produce values less than equal to count, 
	//produced value should get assigned to global variable 'n'.

	while(i<=count)
	{
		n++;
		wait(consumed);
		printf("Produced:%d \n",n);
		i++;
		signal(produced);
	}
}
