#include <cstdio>
#include <omp.h>
#include <cmath>
#include <fstream>

using namespace std;

typedef unsigned long uL;
const int max_value=40000000;



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
//jednokrotny pobieranie do pamięci podręcznej

double sieve_parallel_v() {
		uL i;
        uL p_num_count;
		const uL cache=6*1024*1024;
		int n=sqrt((double) max_value);
        bool * tab = new bool[max_value+1];
        for(i=0;i<=max_value;++i) tab[i]=false;
        double start = omp_get_wtime();
		
		for(uL k=0;k<max_value+cache;k+=cache) {
        #pragma omp parallel default(none) shared(tab,n,k) private(i)
        {
                #pragma omp for schedule(static, 2)
                	for (i=2; i<=n; ++i) {
                    	    if(tab[i] == false) {
									uL j=0;
									if(k!=0) 
										j=k+(i-k%i);
									else 
										j=i+i;
                        	        for(;j<=max_value&&j<=k+cache;j+=i) {
                            	            tab[j] = true;
                               	 }
                        	}
                	}
		}
        }
		for(int i=cache;i<2*cache;++i)
				if(!tab[i]) printf("%d ",i);
		delete [] tab;
        return omp_get_wtime() - start;
       
}


int main(int argc, char **argv) {
		fstream fd; 
		fd.open("time.txt",fstream::in|fstream::out);
		//printf("%f\n", sieve_sequential());
		//printf("%f\n", sieve_parallel());
		//printf("%f\n", sieve_parallel_v());
		fd <<sieve_parallel_v() <<endl;
		fd.close();
        return 0;
}
