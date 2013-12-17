#include<cstdio>
#include<cmath>
#include<omp.h>

typedef unsigned long uL;

const uL N=100;
const uL S=(int)sqrt(N);

uL primes1[N];
uL primes2[N];
uL p_num_count=0;

void genDividers() {
	uL a[S+1];
	#pragma omp paralell for default(none) shared(a) //tylko do pierwszego for'a
	for(uL i=2;i<=S;++i) a[i]=1;
	for(uL i=2;i<=S;++i) { //sito wyznacza dzielniki
		if(a[i]==1) {
			primes1[p_num_count]=i;
			primes2[p_num_count]=i;
			++p_num_count;
			for(uL k=i+i;k<=S;k+=i) a[k]=0;
		}
	}
}

void sequiental() {
	uL rest,i,k,deviders=p_num_count;
	uL count=p_num_count;
	double start=omp_get_wtime();
	for(uL i=S+1;i<=N;++i){
		for(k=0;k<deviders;++k){
			rest=(i%primes2[k]);
			if(!rest) break;
		}
		if(rest) {
			primes2[count++]=i;
		}
	}
	double end=omp_get_wtime();
	printf("\n sequiental %lf \n",end-start);
}

void parallel() {
	double start=omp_get_wtime();
	uL rest,i,k,deviders=p_num_count;
	uL count=p_num_count;
	#pragma omp parallel for default(none) private(rest,k) shared(primes1,count,deviders)
	for(uL i=S+1;i<=N;++i){
		for(k=0;k<deviders;++k){
			rest=(i%primes1[k]);
			if(!rest) break;
		}
		if(rest) {
			#pragma omp critical
			primes1[count++]=i;
		}
	}
	double end=omp_get_wtime();
	printf("\n parallel %lf \n",end-start);
	for(uL i=0;i<count;++i)
			printf("%lu ",primes1[i]);
}




int main(int argc,char **argv) {
	genDividers();
	sequiental();
	parallel();
	return 0;
}
