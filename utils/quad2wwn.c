
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	unsigned long long num;

//	printf("argc: %d\n", argc);
	if (argc < 2) return 1;
	num = strtoull(argv[1],0,10);
	printf("%llx\n", num);
	return 0;
}
