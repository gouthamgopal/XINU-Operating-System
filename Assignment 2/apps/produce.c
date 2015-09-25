 #include <prodcons.h>

 void producer(int count)
 {     

	int32 i;
	
	for(i = 1; i <= count; i++)	{

		n++;
		printf("Produced : %d\n", n);
		
	}
 }
