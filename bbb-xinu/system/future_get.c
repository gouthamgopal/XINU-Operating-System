#include <future.h>

syscall future_get(future* fut, int* value)
{
	if(fut->state == FUTURE_VALID)            //already produced, read the value
	{
		*value = fut->value;         
		return 1;	
	}
	else 
		if(fut->state == FUTURE_EMPTY)       //future is empty, wait for it to be produced
		{
			fut->state = FUTURE_WAITING;      
			fut->pid = currpid;  
                	return 0;
		}
		else 
			if(fut->state == FUTURE_WAITING)
				return -1;
}
