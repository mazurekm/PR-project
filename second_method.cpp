
#include <cstdio>
#include <omp.h>
#include <cmath>
#include <Windows.h>

typedef unsigned long uL;
const int max_value=1000000;


/* sieve of Eratosthenes - sequelntial */

double sieve_sequential() {
        bool * tab = new bool[max_value+1];
        for(uL i=0;i<=max_value;++i) tab[i]=false;
        double start = omp_get_wtime();
        for (uL i=2; i<=max_value; ++i) {
                if(tab[i] == false) {
                        for(uL j=i+i;j<= max_value;j+=i)
                                tab[j] = true;
                }
        }
		delete [] tab;
        return omp_get_wtime() - start;
}

/* sieve of Eratosthenes - parallel */

double sieve_parallel() {
	long i;
        uL p_num_count;
        bool * tab = new bool[max_value+1];
        for(i=0;i<=max_value;++i) tab[i]=false;
        double start = omp_get_wtime();
		
        #pragma omp parallel default(none) shared(tab) firstprivate(i)
        {
                #pragma omp for schedule(static, 2)
                for (i=2; i<=max_value; ++i) {
                        if(tab[i] == false) {
                                for(uL j=i+i;j<=max_value;j+=i) {
                                        tab[j] = true;
                                }
                        }
                }
        }
		delete [] tab;
        return omp_get_wtime() - start;
       
}


int main(int argc, char **argv) {
		//printf("%f", sieve_sequential());
		printf("%f", sieve_parallel());
        return 0;
}
