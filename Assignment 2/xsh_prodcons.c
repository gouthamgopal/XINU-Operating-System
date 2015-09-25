#include <ctype.h>
#include <prodcons.h>


int n;                 
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

      int count = 2000;             //local varible to hold count
	n = 0;
	
	
	if (nargs == 2) {		
		
		if(strncmp(args[1], "--help", 7) == 0)	{
			printf("\nThis command executes producer & consumer!\n");
	}else	{
		if(isNumber(args[1]) == 1)	{
		
			count = atoi(args[1]);	
			//create the process producer and consumer and put them in ready queue.
		  	//Look at the definations of function create and resume in exinu/system folder for reference.      
      			resume( create(producer, 1024, 20, "producer", 1, count) );
      			resume( create(consumer, 1024, 20, "consumer", 1, count) );
			
		}else	{

				printf("\nIncorrect value entered! Please inter the numeric value.\n");
			}
		
		
	}}else if(nargs == 1)	{

			resume( create(producer, 1024, 20, "producer", 1, count) );
      			resume( create(consumer, 1024, 20, "consumer", 1, count) );
	} else	{

		printf("\nMore than one parameters are not allowed!\n");
	}
	

      
}

