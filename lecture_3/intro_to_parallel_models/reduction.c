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
	long long sum = 0;

	struct timeval tstart, tend;
	gettimeofday(&tstart, NULL);
	#pragma omp parallel for reduction(+: sum) 
	for(i=0;i<ARRAY_SIZE;++i) {
		sum += a[i] + b[i];
	}
	gettimeofday(&tend, NULL);
	printf("Sum is :%ld\n",sum);
	return 0;
}
