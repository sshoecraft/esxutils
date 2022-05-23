
#include <stdio.h>
#include <pthread.h>

#define COUNT 10

void *test(void *arg) {
	sleep(10);
	pthread_exit(0);
}

int main(void) {
	pthread_t threads[COUNT];
	int i;

	for(i=0; i < COUNT; i++) pthread_create(&threads[i], 0, test, 0);
	for(i=0; i < COUNT; i++) pthread_join(threads[i],0);
	return 0;
}
