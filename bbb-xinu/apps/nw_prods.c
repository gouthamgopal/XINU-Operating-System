#include<prodcons.h>

uint nw_prods(future *fut) {

	/*int i, j;
	j = (int)fut;

	for (i=0; i<1000; i++) {
		j += i;
	}
	int result = future_set(fut, &j);
	return result;*/
	printf("in prods");
	int i, j;
	j = netserver();
	
	future_set(fut, &j);
	return OK;
}