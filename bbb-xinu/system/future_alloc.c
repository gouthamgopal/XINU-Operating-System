#include <future.h>

future* future_alloc(int future_flags)
{
	future* fut; //create pointer to a future, later to be assigned getmem in order to allocate resources
    		
    	fut = (future*) getmem(sizeof(future));
 
       	if (SYSERR == (int)fut) 	//check if SYSERR is returned
		return NULL;
	else
	{
		fut->state = FUTURE_EMPTY;
		fut->flag = future_flags;
		return fut;
	} 
   	
}
