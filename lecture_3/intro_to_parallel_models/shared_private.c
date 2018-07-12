#include<stdio.h>
#include<omp.h>
#include<sys/time.h>
#include<unistd.h>

#define ARRAY_SIZE 1024768
int main(int argc, char *argv[]) {

	int i;
	int *a = (int *) malloc(sizeof(int) * ARRAY_SIZE);
	int *b = (int *) malloc(sizeof(int) * ARRAY_SIZE);
	int *c = (int *) malloc(sizeof(int) * ARRAY_SIZE);

	struct timeval tstart, tend;
	gettimeofday(&tstart, NULL);
	#pragma omp parallel for shared(a,b,c) private(i) 
	for(i=0;i<ARRAY_SIZE;++i) {
		c[i] = a[i] + b[i];
	}
	gettimeofday(&tend, NULL);
	printf("Time taken is:%d\n",(tend.tv_usec - tstart.tv_usec) 
			+ (tend.tv_sec - tstart.tv_sec) * 1000000);
	return 0;
}
