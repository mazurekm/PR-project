#include <cstdio>
#include <omp.h>
#include <cmath>


typedef unsigned long uL;
const int max_value=100;


/* sieve of Eratosthenes - sequelntial */

void sieve_sequential() {
	bool tab[max_value+1];
	for(uL i=0;i<=max_value;++i) tab[i]=false;
	double start = omp_get_wtime();
	for (uL i=2; i<=max_value; ++i) {
		if(tab[i] == false) { 
			for(uL j=i+i;j<= max_value;j+=i)
				tab[j] = true;
		}
	}
	double end = omp_get_wtime();
	printf("\nSieve of Eratosthenes - sequelntial: %f\n", end - start);
}

/* sieve of Eratosthenes - parallel */

void sieve_parallel() {
	uL i,p_num_count;
	bool tab[max_value+1];
	for(i=0;i<=max_value;++i) tab[i]=false;
	double start = omp_get_wtime();
	#pragma omp parallel default(none) shared(tab) private(i)
	{
		#pragma omp for
		for (i=2; i<=max_value; ++i) {
			if(tab[i] == false) {
				for(uL j=i+i;j<=max_value;j+=i) {
					tab[j] = true;
				}
			}
		}
	}
	double end =omp_get_wtime();
	printf("\nSieve of Eratosthenes - parallel: %f\n", end - start);
	for(i=2;i<=max_value;++i)
			if(!tab[i])
				printf("%lu ",i);
}


int main(int argc, char **argv) {
	sieve_sequential();
	sieve_parallel();
	return 0;
}
