#include <prodcons.h>

void consumer(sid32 produced, sid32 consumed, int count)
{
	int i=1;
	//Code to consume values of global variable 'n' until the value of n is less than or equal to count	
	while(i<=count)
	{
		wait(produced);
		printf("Consumed:%d \n", n);
		i++;
		signal(consumed);
	}

	semdelete(produced);
	semdelete(consumed);
}
