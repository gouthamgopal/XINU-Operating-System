#include<future.h>

unsigned int future_prod(future *fut) {

	int i, j;
	j = (int)fut; //takes only the value of the future passed
	for (i=0; i<1000; i++)    
		j += i;

	printf("Produced: %d \n",j);
	future_set(fut, j); 
	return OK;
}
