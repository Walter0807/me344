#include <iostream>
#include<vector>
#include<stdlib.h>
#include<string>

using namespace std;

class Solution {
 public:
   //the image is x X y pixels
   void matrixMultiply(int *a, int* b, int *c, int n) {  
	int i,j,k;
	for(i=0;i<n;++i) {
		for(j=0;j<n;++j) {
			for(k=0;k<n;++k) {
				c[i*n+j] += a[i*n+k]*b[k*n+j];
			}
		}
	}
   }  


   void matrixMultiply_permute(int *a, int* b, int *c, int n) {  
	//permute the loops i,j,k
	//which one is the best ?
   }

   void matrixMultiply_openmp(int *a, int* b, int *c, int n) {  
	//If you can parallelize only one loop, 
	//using Vtune identify which one you would parallelize ?
	//please explain the numbers that you saw in vtune 
	//and hence, why you chose a specific loop
   }

   void matrixMultiply_tiled(int *a, int* b, int *c, int n) {  
	//please explain the numbers that you saw in vtune 
	//and if so, explain why this version is better than
	//the naive matrixMultiply
	//use large values of n for this
	//and direct the display from standard out to a file
	int i,j,k,jj,kk;
	for(i=0;i<n;++i) {
	  for(j=0;j<n;j+=64) {
	    for(k=0;k<n;k+=64) {
	      for(jj=j;jj<j+64;++jj) {
	  	for(kk=j;kk<k+64;++kk) {
	  		c[i*n+jj] += a[i*n+kk]*b[kk*n+jj];
	        }
	      }
	    }
	  }
	} 
   }

   void display(int *c, int n) {
	int i,j;
	std::cout<<std::endl;
	for(i=0;i<n;++i) {
		for(j=0;j<n;++j) {
			std::cout<<c[i*n+j]<<"\t";
		}
		std::cout<<std::endl;
	}
   }
};

int main(int argc, char *argv[]) {
  Solution hello;
  int n = atoi(argv[1]);
  int *a = (int *)malloc(sizeof(int)*n*n);
  int *b = (int *)malloc(sizeof(int)*n*n);
  int *c = (int *)malloc(sizeof(int)*n*n);

  for(int i=0;i<n;++i) {
    for(int j=0;j<n;++j) {
	a[i*n+j] = rand()%4;
	b[i*n+j] = rand()%4;
        c[i*n+j] = 0;
    }
  }

  hello.display(a, n); 
  hello.display(b, n); 
#define TILED
#ifdef NORMAL
  hello.matrixMultiply(a, b, c, n);
  hello.display(c, n); 
#endif
#ifdef PERMUTE 
  hello.matrixMultiply_permute(a, b, c, n);
  hello.display(c, n); 
#endif
#ifdef OPENMP 
  hello.matrixMultiply_openmp(a, b, c, n);
  hello.display(c, n); 
#endif
#ifdef TILED 
  //ONLY n=MULTIPLE OF 64 
  hello.matrixMultiply_tiled(a, b, c, n);
  hello.display(c, n); 
#endif
}

