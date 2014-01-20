#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <cmath>
#include <time.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

/* generating primes - http://edu.i-lo.tarnow.pl/inf/alg/001_search/0013.php */

bool* generate_primes(int max_value) {
	int num = (int)(sqrt(1.0f*max_value));

	int g, x, y, xx, yy, z, i;
	bool *tab = (bool*) calloc (num+1, sizeof(bool));

	g = (int)(sqrt(1.0f*num));
	for(x = 1; x <= g; ++x)    {
		xx = x * x;
		for(y=1; y<=g; y++)
		{
			yy = y * y;
			z = (xx << 2) + yy;
			if((z <= num) && ((z % 12 == 1) || (z % 12 == 5))) tab[z] = !tab[z];
			z -= xx;
			if((z <= num) && (z % 12 == 7)) tab[z] = !tab[z];
			if(x > y)
			{
				z -= yy << 1;
				if((z <= num) && (z % 12 == 11)) tab[z] = !tab[z];
			}
		}
	}
	for(i=5; i<=g; ++i)
		if(tab[i])
		{
			xx = i * i;
			z = xx;
			while(z <= num)
			{
				tab[z] = false;
				z += xx;
			}
		}
	tab[2] = tab[3] = 1;

	return tab;
}

/* division by primes less then sqrt - sequential */

void division_sequential(int max_value, bool* primes, ofstream &f) {
	double start = omp_get_wtime();

	bool *result = (bool*) calloc (max_value, sizeof(bool));
	int i, j;

	result[2] = result[3] = 1;
	for(i=4; i<=max_value; ++i) {
		bool prime = true;
		int s = (int) sqrt(1.0f * i);
		for(j=2; (j<=s+1) && (prime == true); ++j){
			if ((primes[j] == true) && (i % j == 0))
				prime = false;
		}

		result[i] = prime;
	}

	double stop = omp_get_wtime();
	cout << "Division by primes less then sqrt - sequential: " << stop - start << endl;
	//f << stop - start << endl;

	for (int i=0; i<max_value; ++i)
		if(result[i] == true) {
			f << i << endl;
			//cout << i << " ";
		}
	cout << endl;
}

/* division by primes less then sqrt - parallel (a lot of accesses to memory) */

void division_parallel(int max_value, bool* primes, ofstream &f) {

	double start = omp_get_wtime();

	bool *result = (bool*) calloc (max_value, sizeof(bool));
	int i;
	result[2] = result[3] = 1;

#pragma omp parallel for schedule(dynamic,1) shared(primes, max_value, result)
	for(i=4; i<=max_value; ++i) {
		bool prime = true;
		int s = (int) sqrt(1.0f * i);
		int j;
		for(j=2; (j<=s+1) && (prime == true); ++j){
			if ((primes[j] == true) && (i % j == 0))
				prime = false;
		}
		result[i] = prime;
	}

	double stop = omp_get_wtime();
	cout << "Division by primes less then sqrt - parallel: " << stop - start << endl;
	//f << stop - start << endl;

	for (int i=0; i<max_value; ++i)
		if(result[i] == true) {
			f << i << endl;
			//cout << i << " ";
		}
	cout << endl;
}

/* division by primes less then sqrt - parallel (one access to memory) */

void division_parallel_one(int max_value, bool* primes, ofstream &f) {
	int num = 4;
	omp_set_num_threads(num);
	double start = omp_get_wtime();

	bool *result = (bool*) calloc (max_value, sizeof(bool));
	result[2] = result[3] = 1;

	int pp = 6 * 1024 * 1024;

	int toAnalyze = max_value;
	int beginAllThr = 0;
	int forOne;

	while (toAnalyze > 0) {

		if(toAnalyze > pp) {
			forOne = pp/num;
			toAnalyze = toAnalyze - pp;
		}
		else {
			forOne = toAnalyze / num;
			toAnalyze = 0;
		}
		int begin = 0, end = 0;
		int j;
#pragma omp parallel shared(result, primes, beginAllThr, forOne) private(begin,end)
		{
		#pragma omp for schedule(static,1)
		for(int i=0; i<num; ++i)
		{
			begin = beginAllThr + i * forOne + 1;
			if((i == 0) && (beginAllThr == 0))
				begin = 4;
			end = beginAllThr + (i + 1) * forOne;
			printf("Watek: %d\nprziedział: %d - %d\n\n", omp_get_thread_num(), begin, end);

			for(j=begin; j<=end; ++j) {
				bool prime = true;
				for(int k=2; (k<=sqrt(1.0f * j)) && (prime == true); ++k) {
					if((primes[k] == true) && (j % k == 0)) {
						prime = false;
						//printf("Odrzucane: %d - dzieli sie przez %d\n", j, k);
					}
				}
				result[j] = prime;
			}
		}
		}
		beginAllThr = beginAllThr + num * forOne;
	}
	double stop = omp_get_wtime();
	cout << "Division by primes less then sqrt - parallel (one access): " << stop - start << endl;
	//f << stop - start << endl;

	for (int i=0; i<max_value; ++i)
		if(result[i] == true) {
			f << i << endl;
			//cout << i << " ";
		}
	cout << endl;
}

/* sieve of Eratosthenes - sequelntial */

void sieve_sequential(int max_value, ofstream &f) {
	//init table - clean
	bool *tab = (bool*) calloc (max_value, sizeof(bool));
	int s = (int) sqrt(1.0f*max_value);

	//0 and 1 are not primes
	tab[0] = tab[1] = 1;

	//deleting multiples
	double start = omp_get_wtime();

	int i;
	for (i=2; i<=s+1; ++i) {
		if(tab[i] == false) {
			for(int j=i*i; j<max_value ; j=j+i) 
				tab[j] = 1;	
		}
	}
	double stop = omp_get_wtime();
	cout << "Sieve of Eratosthenes - sequelntial: " << stop - start << endl;
	//f << stop - start << endl;

	for (int i=0; i<max_value; ++i)
		if(tab[i] == false) {
			f << i << endl;
			//cout << i << " ";
		}
	cout << endl;

}


/* sieve of Eratosthenes - parallel (a lot of accesses to memory) */

void sieve_parallel(int max_value, ofstream &f) {
	//init table - clean
	bool *tab = (bool*) calloc (max_value, sizeof(bool));
	int s = (int) sqrt(1.0f*max_value);

	//0 and 1 are not primes
	tab[0] = tab[1] = 1;

	//deleting multiples
	double start = omp_get_wtime();

#pragma omp parallel shared(tab, max_value)
	{
		int i;
#pragma omp for ordered schedule(dynamic)
		for (i=2; i<=s; ++i) {
			//printf("Wątek: %d\ti: %lu\n", omp_get_thread_num(), i);
			if(tab[i] == false) {
				for(int j=i*i; j<max_value ; j=j+i)
					tab[j] = 1;
			}
		}
	}

	double stop = omp_get_wtime();
	cout << "Sieve of Eratosthenes - parallel: " << stop - start << endl;
	//f << stop - start << endl;

	for (int i=0; i<max_value; ++i)
		if(tab[i] == false) {
			f << i << endl;
			//cout << i << " ";
		}
	cout << endl;
}


/* sieve of Eratosthenes - parallel (one access to memory) */

void sieve_parallel_one(int max_value, ofstream &f) {
	//init table - clean
	bool *tab = (bool*) calloc (max_value, sizeof(bool));
	int s ;//= (int) sqrt(1.0f*max_value);

	//0 and 1 are not primes
	tab[0] = tab[1] = 1;

	double start = omp_get_wtime();
	int pp = 6 * 1024 * 1024;
	int k;

	int max;
	for (max=pp; max<max_value+pp; max=max+pp) {
#pragma omp parallel shared(tab, max_value)
		{
			int i;
			s = (int) sqrt(1.0f*max);
#pragma omp for ordered schedule(dynamic)
			for (i=2; i<=s; ++i) {

				if(tab[i] == false) {
					if (max == pp)
						k = i*i;
					else 
						k = (max - pp) + i - ((max - pp) % i);
					for(int j=k; j<=max && j<max_value ; j=j+i) 
						tab[j] = 1;
				}
			}
		}
	}
	double stop = omp_get_wtime();
	cout << "Sieve of Eratosthenes - parallel (one access): " << stop - start << endl;
	//f << stop - start << endl;

	for (int i=0; i<max_value; ++i)
		if(tab[i] == false) {
			f << i << endl;
			//cout << i << " ";
		}
	cout << endl;
}

int main(int argc, char **argv) {
	omp_set_num_threads(4);

	int max = 8000000;
	/*	cout << "Max number: ";
		cin >> max;*/
	string max_str;
	stringstream ss;
	ss << max;
	ss >> max_str;
	ofstream f(("result" + max_str + ".txt").c_str());
	ofstream f1("r1");
	ofstream f2("r2");

	bool* primes = generate_primes(max);

	//	division_sequential(max, primes, f2);
	//	division_parallel(max, primes, f2);
	division_parallel_one(max, primes, f2);

	//sieve_sequential(max, f1);
	//	sieve_parallel(max, f2);
	//	sieve_parallel_one(max, f2);

	f.close();

	//system("PAUSE");
	return 0;
}
