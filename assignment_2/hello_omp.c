#include <stdio.h>
#include <omp.h>

int main() {

#pragma omp parallel
{
#pragma omp single
  {
	printf("\nhello world ");
    printf("with %d threads out of %d available \n\n",
      omp_get_num_threads(), omp_get_num_procs());
  }
  printf("hello from thread number %d \n", omp_get_thread_num());
  fflush(stdout);
}
printf("\ngoodbye world \n\n");
return 0;
}