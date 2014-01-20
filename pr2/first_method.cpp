// met1.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"


#include<cstdio>
#include<cmath>
#include<omp.h>
//#include<Windows.h>

typedef unsigned long uL;

const uL N=20000000;
const uL S=(int)sqrt((float)N);

uL primes1[N];
uL primes2[N];
uL p_num_count=0;

void genDividers() {
		uL * a = new uL [S+1];
        #pragma omp parallel for default(none) shared(a) //tylko do pierwszego for'a
        for(long i=2;i<=S;++i) a[i]=1;
        for(uL i=2;i<=S;++i) { //sito wyznacza dzielniki
                if(a[i]==1) {
                        primes1[p_num_count]=i;
                        primes2[p_num_count]=i;
                        ++p_num_count;
                        for(uL k=i+i;k<=S;k+=i) a[k]=0;
                }
        }
		delete [] a;
}

double sequiental() {
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
        return omp_get_wtime() - start;
}

double parallel() {
        uL rest,i,k,deviders=p_num_count;
        uL count=p_num_count;
		double start=omp_get_wtime();
		#pragma omp parallel 
		{
			DWORD_PTR mask = (1 << omp_get_thread_num());
			SetThreadAffinityMask(GetCurrentThread(), mask);
		}
        #pragma omp parallel for default(none) firstprivate(rest,k) shared(primes1,count,deviders)
        for(long i=S+1;i<=N;++i){
                for(k=0;k<deviders;++k){
                        rest=(i%primes1[k]);
                        if(!rest) break;
                }
                if(rest) {
                        #pragma omp critical
                        primes1[count++]=i;
                }
        }
        return omp_get_wtime() - start;

}




int main(int argc,char **argv) {
        genDividers();
		//printf("%f\n", sequiental());
		printf("%f", parallel());
        return 0;
}

