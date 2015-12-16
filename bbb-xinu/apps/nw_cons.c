#include<prodcons.h>

uint nw_cons(future *fut) {

	printf("cons");
	/*int i, status;

	status = future_get(fut, &i);

	if (status < 1) {
		printf("future_get failed\n");
		return -1;
	}
	
	printf("it produced %d (Consumer PID : %d)\n", i, currpid);
	if(fut->flag == FUTURE_EXCLUSIVE){
		future_free(fut);				
		
	}else if(fisempty(fut->get_queue)){
		future_free(fut);
	}
	
	return OK;*/

int i, status;

	
	status = future_get(fut, &i);

	if (status < 1) {
		printf("future_get failed\n");
		return -1;
	}
	
	printf("It produced : %d \n", i);
	if(fut->flag == FUTURE_EXCLUSIVE){
		
		future_free(fut);		
		
		return OK;
	}
	if(fisempty(fut->get_queue)){
		
		future_free(fut);
		
	}
	
	return OK;


}
