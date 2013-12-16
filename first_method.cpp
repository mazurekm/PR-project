#include<cstdio>
#include<cmath>
#include<omp.h>

const int N=10000000;
const int S=sqrt(N);


int main(int argc,char **argv) {
	long a[S+1];
	long primes[N/10];
	long p_num_count=0,k=0,rest=0;

	#pragma omp parallel for default(none) shared(a)
	for(long i=2;i<=S;++i) a[i]=1;
	for(long i=2;i<=S;++i) { //sito wyznacza dzielniki
		if(a[i]==1) {
			primes[p_num_count++]=i;
			for(k=i+1;k<=S;k+=i) a[k]=0;
		}
	}

	long deviders=p_num_count;

	#pragma omp parallel for default(none) private(k,rest) shared(primes,p_num_count,deviders)
	for(long i=S+1;i<=N;++i){
		for(k=0;k<deviders;++k){
			rest=(i%primes[k]);
			if(!rest) break;
		}
		if(rest) {
			#pragma omp critical
			primes[p_num_count++]=i;
		}
	}
	return 0;
}
