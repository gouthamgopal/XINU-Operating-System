#include <future.h>

syscall future_set(future* fut, int value)
{
	
	if(fut->state == FUTURE_EMPTY)	 			//if empty, assign value and set to valid state
	{
		fut->state = FUTURE_VALID;
		fut->value= value;		
	}	
	else 
		if(fut->state == FUTURE_WAITING)	 	//if waiting, assign value and set to valid state	
		{
                	fut->state = FUTURE_VALID;	
			fut->value = value;			
		}
		else 
			if(fut->state == FUTURE_VALID)	 	//if valid, you can't call set again, so return error
				return SYSERR;
	return OK;
}
