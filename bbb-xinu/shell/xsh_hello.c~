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


	if (nargs > 1) {
		printf("Hello %s", args[1]);

		for (i = 2; i < nargs; i++) {
			printf("%s", args[i]);
		}
		printf(".....Welcome to Xinu !! !! !!");
	}
	else
		printf("!! !! !! Hi !! !! !!");
	printf("\n");

	return 0;
}
