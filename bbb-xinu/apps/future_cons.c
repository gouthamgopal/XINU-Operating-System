#include <future.h>

unsigned int future_cons(future *fut) {

int i, status=0;

   while(1)
   {
	//status gives the current status of the future that is passed
	status = future_get(fut, &i);
	if (status < 1)
	   continue;
	else 
		if (status == 1) 
		{
			printf("Consumed: %d \n",i);
    			break;
  		}
    }     
    return OK;
}
