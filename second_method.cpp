#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <vector>

/* sieve of Eratosthenes - sequelntial */

void sieve_sequential(unsigned long max_value) {
	//init table - clean
	bool *tab = (bool*) calloc (max_value, sizeof(bool));

double start = omp_get_wtime();

	//deleting multiples
	for (unsigned long i=2; i<max_value; ++i) {
		if(tab[i] == false) { 
			//printf("%ld ", i);
			unsigned long j = i;
			while(j+i < max_value) {
				j = j + i;
				tab[j] = 1;
			}
		}
	}
        printf("\nSieve of Eratosthenes - sequelntial: %f\n", omp_get_wtime() - start);

	/*for (unsigned long i=2; i<max_value; ++i)
		if(tab[i] == false)
			printf("%lu ", i);
	printf("\n");*/

}


/* sieve of Eratosthenes - parallel */

void sieve_parallel(unsigned long max_value) {
	//init table - clean
	bool *tab = (bool*) calloc (max_value, sizeof(bool));
	//deleting multiples
	unsigned long i, j;
	//omp_set_num_threads(4);
	double start = omp_get_wtime();
	#pragma omp parallel default(none) shared(tab,max_value) private(i,j)
	{
		#pragma omp for
		for (i=2; i<max_value; ++i) {
			if(tab[i] == false) { 
				//printf("%ld ", i);
				for(j=i+i;j<max_value;j+=i) {
					tab[j] = true;
				}
			}
		}
	}

        printf("\nSieve of Eratosthenes - parallel: %f\n", omp_get_wtime() - start);

	for (unsigned long i=2; i<max_value; ++i)
		if(tab[i] == false)
			printf("%lu ", i);
	printf("\n");

}


int main(int argc, char **argv) {
	//omp_set_num_threads(3);
	sieve_sequential(100000);
	sieve_parallel(100000);
	return 0;
}
