#include <ctype.h>
#include <prodcons.h>

int n;
sid32 produced, consumed;                 
/*Now global variable n will be on Heap so it is accessible all the processes i.e. consume and produce*/

int isNumber(const char *val)	{	
	while(*val != '\0')	{
		if(*val < '0' || *val > '9')	{
			return 0;
		}
		val++;
	}
	return 1;
}

shellcmd xsh_prodcons(int nargs, char *args[])
{
      //Argument verifications and validations
	int count=0;             //local varible to hold count	
	
	if (nargs == 2) {		
		if(strncmp(args[1], "--help", 7) == 0)	{
			printf("\nThis command executes producer & consumer!\n");
		}else if(strncmp(args[1], "-f", 3) == 0) {

			future *f1, *f2, *f3;
 			f1 = future_alloc(FUTURE_EXCLUSIVE);
	  		f2 = future_alloc(FUTURE_EXCLUSIVE);
	  		f3 = future_alloc(FUTURE_EXCLUSIVE);

			resume( create(future_cons, 1024, 20, "fcons1", 1, f1) );
			resume( create(future_prod, 1024, 20, "fprod1", 1, f1) );
			resume( create(future_cons, 1024, 20, "fcons2", 1, f2) );
			resume( create(future_prod, 1024, 20, "fprod2", 1, f2) );
			resume( create(future_cons, 1024, 20, "fcons3", 1, f3) );
			resume( create(future_prod, 1024, 20, "fprod3", 1, f3) );
		} else if(args[1] > 0) {
			/*Initialise semaphores*/
			consumed = semcreate(1);      
			produced = semcreate(0);
			count = atoi(args[1]);				
			//create the process producer and consumer and put them in ready queue.
			resume( create(consumer, 1024, 20, "consumer", 3, produced, consumed, count) );
			resume( create(producer, 1024, 20, "producer", 3, produced, consumed, count) );	
		}
	
	} else {
		printf("Invalid argument\n");
		return 1;
	}    	  
}
