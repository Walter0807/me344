#include <stdio.h>
#include <omp.h>
int main() {
  #pragma omp parallel  //CREATE A PARALLEL REGION
  {
    printf("\nhello world ");
    printf("hello from thread number %d \n",              
                        omp_get_thread_num());
    fflush(stdout);
  }
  printf("\ngoodbye world \n\n");
  return 0;
}
