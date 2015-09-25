/*
Authors: aykaranj(Abhijit Karanjkar), 
	 ronair(Rohit Nair)
*/
/* xsh_hello.c - xsh_hello */


#include <xinu.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xsh_hello - Xinu says Hello for the 1st time
 *------------------------------------------------------------------------
*/ 

shellcmd xsh_hello(int nargs, char *args[])
{
	int32	i;			/* walks through args array	*/


	if (nargs == 2) {
		printf("Hello %s", args[1]);		
		printf("\n****** Welcome to Xinu ******");
	}
	else if(nargs < 2)	{
		printf("No argument given!");
	} else if(nargs > 2)	{
		printf("Too many arguments!");
	}
	

	return 0;
}
