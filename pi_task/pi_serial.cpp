#include <iostream>
#include <omp.h>
#include <fstream>
#include <windows.h>
#include <vector>

using namespace std;

const long long num_steps = 1000000000;
double step;

double sequential() {
	double start, stop;
	double x, pi, sum=0.0;
	int i;
	step = 1./(double)num_steps;
	start=omp_get_wtime();
	for (i=0; i<num_steps; i++)
	{
		x = (i + .5)*step;
		sum = sum + 4.0/(1.+ x*x);
	}
	
	pi = sum*step;
	stop = omp_get_wtime();

	cout <<"Wartosc liczby PI wynosi " <<pi <<endl;
	cout <<"Czas przetwarzania wynosi " <<stop-start <<endl;
	return stop-start;
}

double versionOne() {
	double start, stop;
	double x, pi, sum=0.0;
	int i;
	step = 1./(double)num_steps;
	start = omp_get_wtime();
	#pragma omp parallel for private(i,x) shared(sum,step)
	for (i=0; i<num_steps; i++)
	{
		x = (i + .5)*step;
		#pragma omp atomic
			sum += 4.0/(1.+ x*x);
	}
	
	pi = sum*step;
	stop = omp_get_wtime();

	cout <<"Wartosc liczby PI wynosi " <<pi <<endl;
	cout <<"Czas przetwarzania wynosi " <<stop-start <<endl;
	return stop-start;
}

double versionTwo() {
	double start, stop;
	double x, pi, sum=0.0;
	int threads=4;
	omp_set_dynamic(0);
	omp_set_num_threads(threads);
	int i;
	step = 1./(double)num_steps;
	start = omp_get_wtime();
	#pragma omp parallel private(i,x) shared(step) 
	{
		/*HANDLE th_handle=GetCurrentThread(); //do zadania 3.8
		int th_id=omp_get_thread_num(); 
		DWORD_PTR mask = (1 << (th_id % 2 ));
		SetThreadAffinityMask(th_handle,mask);*/
		#pragma omp for reduction(+:sum)
		for (i=0; i<num_steps; i++)
		{
			x = (i + .5)*step;
			sum = sum + 4.0/(1.+ x*x);
		}
	}
	pi = sum*step;
	stop = omp_get_wtime();

	cout <<"Wartosc liczby PI wynosi " <<pi <<endl;
	cout <<"Czas przetwarzania wynosi " <<stop-start <<endl;
	return stop-start;
}

double versionThree() {
	double start, stop;
	double x, pi, sum=0.0;
	int threads=4;
	omp_set_dynamic(0);
	omp_set_num_threads(threads);
	double *sharedTab=new double [200];	
	int i;
	for(i=0;i<threads;++i)
		sharedTab[i]=0.0;
	step = 1./(double)num_steps;
	start = omp_get_wtime();	
	#pragma omp parallel for private(i,x) shared(step,sharedTab) 
	for (i=0; i<num_steps; i++)
	{
		int j=omp_get_thread_num();
		x = (i + .5)*step;
		sharedTab[j] = sharedTab[j] + 4.0/(1.+ x*x);
	}
	for (i=0; i<threads; i++)
		sum+=sharedTab[i];	
	pi = sum*step;
	stop = omp_get_wtime();

	cout <<"Wartosc liczby PI wynosi " <<pi <<endl;
	cout <<"Czas przetwarzania wynosi " <<stop-start <<endl;
	delete [] sharedTab;
	return stop-start;
}

vector<double> versionFour() { ///do zadania 3.7
	double start, stop;
	vector<double> v;
	double x, pi, sum=0.0;
	int threads=2;
	omp_set_dynamic(0);
	omp_set_num_threads(threads);
	double *sharedTab=new double [200];	
	int i;
	step = 1./(double)num_steps;
	for(int k=0;k<20;++k) {
		for(i=0;i<200;++i)
			sharedTab[i]=0.0;
		start = omp_get_wtime();	
		#pragma omp parallel for private(i,x) shared(step,sharedTab,k) 
		for (i=0; i<num_steps; i++)
		{
			int j=omp_get_thread_num();
			x = (i + .5)*step;
			sharedTab[j+k] = sharedTab[j+k] + 4.0/(1.+ x*x);
		}
		for (i=0; i<threads; i++)
			sum+=sharedTab[i];	
		pi = sum*step;
		stop = omp_get_wtime();
		v.push_back(stop-start);
	}
	//cout <<"Wartosc liczby PI wynosi " <<pi <<endl;
	//cout <<"Czas przetwarzania wynosi " <<stop-start <<endl;
	delete [] sharedTab;
	return v;
}

int main(int argc, char* argv[])
{
	ios_base::sync_with_stdio(0);
	fstream fd; 
	fd.open("8.csv",fstream::in|fstream::out|fstream::app);
	//vector<double> v=versionFour();
	//for(vector<double>::iterator it=v.begin();it!=v.end();++it)
		//fd<< *it <<endl;
	for(int i=0;i<3;++i) {
		//fd << sequential() <<endl;
		//fd << versionOne() <<endl;
		fd << versionTwo() <<endl;
		//fd << versionThree() <<endl;

		//fd << sequential() <<";" <<versionOne() <<";" <<versionTwo() <<";" <<versionThree()  <<endl;
	}
	fd << endl;
	fd.close();
	getchar();
	return 0;
}
