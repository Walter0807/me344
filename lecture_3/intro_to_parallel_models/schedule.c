#include<stdio.h>
#include<omp.h>
#include<sys/time.h>
#include<unistd.h>
#include<math.h>

#define ARRAY_SIZE 1024768
int main(int argc, char *argv[]) {

	int i;
	int n = atoi(argv[1]);
	int *a = (int *) malloc(sizeof(int) * n);
	int *b = (int *) malloc(sizeof(int) * n);
	int *c = (int *) malloc(sizeof(int) * n);
	struct timeval tstart, tend;
	gettimeofday(&tstart, NULL);
	#pragma omp parallel for schedule(static) 
	for(i=0;i<n;++i) {
		c[i] = a[i] + b[i];
	}
	gettimeofday(&tend, NULL);
	printf("Time taken is:%d\n",(tend.tv_usec - tstart.tv_usec) 
			+ (tend.tv_sec - tstart.tv_sec) * 1000000);
	return 0;
}
