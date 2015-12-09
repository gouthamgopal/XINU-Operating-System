 #include <newnet.h>
  /*Now global variables will be on Heap so they are accessible to all the processes i.e. consume and produce*/

   shellcmd xsh_network(int nargs, char *args[])
   {	
	resume( create(netserver, 1024, 20, "netserver", 0) );
	return 0;
   }
