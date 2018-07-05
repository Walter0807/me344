/*
 * Copyright (C) 2j010 - 2015 Intel Corporation. All Rights Reserved.
 *
 * The source code contained or described herein and all
 * documents related to the source code ("Material") are owned by
 * Intel Corporation or its suppliers or licensors. Title to the
 * Material remains with Intel Corporation or its suppliers and
 * licensors. The Material is protected by worldwide copyright
 * laws and treaty provisions.  No part of the Material may be
 * used, copied, reproduced, modified, published, uploaded,
 * posted, transmitted, distributed,  or disclosed in any way
 * except as expressly provided in the license provided with the
 * Materials.  No license under any patent, copyright, trade
 * secret or other intellectual property right is granted to or
 * conferred upon you by disclosure or delivery of the Materials,
 * either expressly, by implication, inducement, estoppel or
 * otherwise, except as expressly provided in the license
 * provided with the Materials
 *
 */

//  Simple minded matrix multiply
#include <stdio.h>
#include <sys/time.h>


#define N 2000000000

double f( double x )
;
double clock_it(void)
{
	double duration = 0.0;
	struct timeval start;

	gettimeofday(&start, NULL);
	duration = (double)(start.tv_sec + start.tv_usec/1000000.0);
	return duration;
}

main()
{
	double sum, pi, x, h;
	double start_time, stop_time;
	 int i;

        h = (double)1.0/(double)N;
        sum = 0.0;

	start_time = clock_it();
#ifdef _OPENMP
#pragma omp parallel for private(x) reduction(+:sum)
#endif
        for ( i=0; i<N ; i++ ){
          x = h*(i-0.5);
          sum = sum + f(x);
        }
	stop_time = clock_it();

	// print value of pi to be sure multiplication is correct
        pi = h*sum;
        printf("    pi is approximately : %12.9f \n", pi);
	// print elapsed time
	printf("Elapsed time = %lf seconds\n",(stop_time - start_time));

}
