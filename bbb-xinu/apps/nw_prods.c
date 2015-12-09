#include<prodcons.h>

uint net_producer(future *fut) {

	int i, j;
	j = (int)fut;

	for (i=0; i<1000; i++) {
		j += i;
	}
	int result = future_set(fut, &j);
	return result;
}
