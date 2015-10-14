#include <future.h>

syscall future_free(future* fut)
{
	int retValue=0;

	//future is passes to freemem to free the memory
	retValue = freemem(fut,sizeof(future));  
   
	if(retValue==SYSERR)
	        return SYSERR;

	return OK;
}
