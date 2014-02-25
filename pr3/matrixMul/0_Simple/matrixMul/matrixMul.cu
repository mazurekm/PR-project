/**
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

/**
 * Matrix multiplication: C = A * B.
 * Host code.
 *
 * This sample implements matrix multiplication as described in Chapter 3
 * of the programming guide.
 * It has been written for clarity of exposition to illustrate various CUDA
 * programming principles, not with the goal of providing the most
 * performant generic kernel for matrix multiplication.
 *
 * See also:
 * V. Volkov and J. Demmel, "Benchmarking GPUs to tune dense linear algebra,"
 * in Proc. 2008 ACM/IEEE Conf. on Superconducting (SC '08),
 * Piscataway, NJ: IEEE Press, 2008, pp. Art. 31:1-11.
 */

// System includes
#include <stdio.h>
#include <assert.h>

// CUDA runtime
#include <cuda_runtime.h>

// Helper functions and utilities to work with CUDA
#include <helper_functions.h>

/**
 * Matrix multiplication (CUDA Kernel) on the device: C = A * B
 * wA is A's width and wB is B's width
 */

//enum version {VERSION1, VERSION2, VERSION3, VERSION4, VERSION5 };

	template <int BLOCK_SIZE> __global__ void
matrixMulv1(float *C, float *A, float *B, int width) {
	int tx=threadIdx.x;
	int ty=threadIdx.y;
	for (int sx = tx; sx < width; sx += BLOCK_SIZE) {
		for (int sy = ty; sy < width; sy += BLOCK_SIZE) { 
			float score=0; 	
			for(int k=0;k<width;++k) {
				float a_element=A[sy*width+k];
				float b_element=B[sx+k*width];
				score+=a_element*b_element;
			}
			C[sy*width+sx]=score;
		}
	}
}

	template <int BLOCK_SIZE> __global__ void
matrixMulv2(float *C, float *A, float *B, int width){
	int row=blockIdx.y*blockDim.y+threadIdx.y;
	int col=blockIdx.x*blockDim.x+threadIdx.x;
	float c_local=0;
	for(int k=0;k<width;++k)
		c_local+=A[row*width+k]*B[k*width+col];
	C[row*width+col]=c_local;	
}

	template <int BLOCK_SIZE> __global__ void
matrixMulv3(float *C, float *A, float *B, int width)
{
	// Block index
	int bx = blockIdx.x;
	int by = blockIdx.y;

	// Thread index
	int tx = threadIdx.x;
	int ty = threadIdx.y;

	// Index of the first sub-matrix of A processed by the block
	int aBegin = width * BLOCK_SIZE * by;

	// Index of the last sub-matrix of A processed by the block
	int aEnd   = aBegin + width - 1;

	// Step size used to iterate through the sub-matrices of A
	int aStep  = BLOCK_SIZE;

	// Index of the first sub-matrix of B processed by the block
	int bBegin = BLOCK_SIZE * bx;

	// Step size used to iterate through the sub-matrices of B
	int bStep  = BLOCK_SIZE * width;

	// Csub is used to store the element of the block sub-matrix
	// that is computed by the thread
	float Csub = 0;

	// Loop over all the sub-matrices of A and B
	// required to compute the block sub-matrix
	for (int a = aBegin, b = bBegin;
			a <= aEnd;
			a += aStep, b += bStep)
	{

		// Declaration of the shared memory array As used to
		// store the sub-matrix of A
		__shared__ float As[BLOCK_SIZE][BLOCK_SIZE];

		// Declaration of the shared memory array Bs used to
		// store the sub-matrix of B
		__shared__ float Bs[BLOCK_SIZE][BLOCK_SIZE];

		// Load the matrices from device memory
		// to shared memory; each thread loads
		// one element of each matrix
		As[ty][tx] = A[a + width * ty + tx];
		Bs[ty][tx] = B[b + width * ty + tx];

		// Synchronize to make sure the matrices are loaded
		__syncthreads();

		// Multiply the two matrices together;
		// each thread computes one element
		// of the block sub-matrix
#pragma unroll

		for (int k = 0; k < BLOCK_SIZE; ++k)
		{
			Csub += As[ty][k] * Bs[k][tx];
		}

		// Synchronize to make sure that the preceding
		// computation is done before loading two new
		// sub-matrices of A and B in the next iteration
		__syncthreads();
	}

	// Write the block sub-matrix to device memory;
	// each thread writes one element
	int c = width * BLOCK_SIZE * by + BLOCK_SIZE * bx;
	C[c + width * ty + tx] = Csub;
}

	template <int BLOCK_SIZE> __global__ void
matrixMulv4(float *C, float *A, float *B, int width)
{
	// Block index
	int bx = blockIdx.x;
	int by = blockIdx.y;

	// Thread index
	int tx = threadIdx.x;
	int ty = threadIdx.y;

	// Index of the first sub-matrix of A processed by the block
	int aBegin = width * BLOCK_SIZE * by;

	// Index of the last sub-matrix of A processed by the block
	int aEnd   = aBegin + width - 1;

	// Step size used to iterate through the sub-matrices of A
	int aStep  = BLOCK_SIZE;

	// Index of the first sub-matrix of B processed by the block
	int bBegin = BLOCK_SIZE * bx;

	// Step size used to iterate through the sub-matrices of B
	int bStep  = BLOCK_SIZE * width;

	// Csub is used to store the element of the block sub-matrix
	// that is computed by the thread
	float Csub = 0;
	int flip=0;
	__shared__ float As[2][BLOCK_SIZE][BLOCK_SIZE];
	__shared__ float Bs[2][BLOCK_SIZE][BLOCK_SIZE];

	As[flip][ty][tx] = A[aBegin + width * ty + tx];
	Bs[flip][ty][tx] = B[bBegin + width * ty + tx];
	__syncthreads();

	// Loop over all the sub-matrices of A and B
	// required to compute the block sub-matrix
	for (int a = aBegin, b = bBegin;
			a <= aEnd;
			a += aStep, b += bStep)
	{
		flip=!flip;
		As[flip][ty][tx] = A[a + width * ty + tx];
		Bs[flip][ty][tx] = B[b + width * ty + tx];

		// Synchronize to make sure the matrices are loaded
		__syncthreads();

		// Multiply the two matrices together;
		// each thread computes one element
		// of the block sub-matrix
#pragma unroll

		for (int k = 0; k < BLOCK_SIZE; ++k)
		{
			Csub += As[!flip][ty][k] * Bs[!flip][k][tx];
		}

		// Synchronize to make sure that the preceding
		// computation is done before loading two new
		// sub-matrices of A and B in the next iteration
		__syncthreads();
	}

	// Write the block sub-matrix to device memory;
	// each thread writes one element
	int c = width * BLOCK_SIZE * by + BLOCK_SIZE * bx;
	C[c + width * ty + tx] = Csub;
}

	template <int BLOCK_SIZE> __global__ void
matrixMulv5(float *C, float *A, float *B, int width)
{
	// Block index
	int bx = blockIdx.x;
	int by = blockIdx.y;

	// Thread index
	int tx = threadIdx.x;
	int ty = threadIdx.y;

	// Index of the first sub-matrix of A processed by the block
	int aBegin = width * BLOCK_SIZE * by;

	// Index of the last sub-matrix of A processed by the block
	int aEnd   = aBegin + width - 1;

	// Step size used to iterate through the sub-matrices of A
	int aStep  = BLOCK_SIZE;

	// Index of the first sub-matrix of B processed by the block
	int bBegin = BLOCK_SIZE * bx;

	// Step size used to iterate through the sub-matrices of B
	int bStep  = 2*BLOCK_SIZE * width;

	// Csub is used to store the element of the block sub-matrix
	// that is computed by the thread
	float Csub1 = 0, Csub2=0;
	int flip=0;
	__shared__ float As[2][BLOCK_SIZE][BLOCK_SIZE];
	__shared__ float Bs[2][BLOCK_SIZE][BLOCK_SIZE];

	As[flip][ty][tx] = A[aBegin + width * ty + tx];
	Bs[flip][ty][tx] = B[bBegin + width * ty + tx];
	Bs[flip][ty][tx+1] = B[bBegin+bStep + width * ty + tx];

	__syncthreads();

	// Loop over all the sub-matrices of A and B
	// required to compute the block sub-matrix
	for (int a = aBegin, b = bBegin;
			a <= aEnd;
			a += aStep, b += bStep)
	{
		flip=!flip;
		As[flip][ty][tx] = A[a + width * ty + tx];
		Bs[flip][ty][tx] = B[b + width * ty + tx];
		Bs[flip][ty][tx+1] = B[b+bStep + width *ty  + tx];

		// Synchronize to make sure the matrices are loaded
		__syncthreads();

		// Multiply the two matrices together;
		// each thread computes one element
		// of the block sub-matrix
#pragma unroll

		for (int k = 0; k < BLOCK_SIZE; ++k)
		{
			Csub1 += As[!flip][ty][k] * Bs[!flip][k][tx];
			Csub2 += As[!flip][ty][k] * Bs[!flip][k][tx+1];
		}

		// Synchronize to make sure that the preceding
		// computation is done before loading two new
		// sub-matrices of A and B in the next iteration
		__syncthreads();
	}

	// Write the block sub-matrix to device memory;
	// each thread writes one element
	int c1 = width * BLOCK_SIZE * by + BLOCK_SIZE * bx;
	C[c1 + width * ty + tx] = Csub1;
	C[c1 + width * ty + tx+1] = Csub2;
}



void constantInit(float *data, int size, float val)
{
	for (int i = 0; i < size; ++i)
	{
		data[i] = val;
	}
}

/**
 * Run a simple test of matrix multiplication using CUDA
 */




int matrixMultiply(int argc, char **argv, int block_size, int width, int v)
{
	// Allocate host memory for matrices A and B
	unsigned int mem_size_A = sizeof(float) * width*width;
	float *h_A = (float *)malloc(mem_size_A);
	unsigned int mem_size_B = sizeof(float) * width*width;
	float *h_B = (float *)malloc(mem_size_B);

	// Initialize host memory
	const float valB = 0.01f;
	constantInit(h_A, width*width, 1.0f);
	constantInit(h_B, width*width, valB);

	// Allocate device memory
	float *d_A, *d_B, *d_C;

	// Allocate host matrix C
	dim3 dimsC(width, width, 1);
	unsigned int mem_size_C = dimsC.x * dimsC.y * sizeof(float);
	float *h_C = (float *) malloc(mem_size_C);

	if (h_C == NULL)
	{
		fprintf(stderr, "Failed to allocate host matrix C!\n");
		exit(EXIT_FAILURE);
	}

	cudaError_t error;

	error = cudaMalloc((void **) &d_A, mem_size_A);

	if (error != cudaSuccess)
	{
		printf("cudaMalloc d_A returned error code %d, line(%d)\n", error, __LINE__);
		exit(EXIT_FAILURE);
	}

	error = cudaMalloc((void **) &d_B, mem_size_B);

	if (error != cudaSuccess)
	{
		printf("cudaMalloc d_B returned error code %d, line(%d)\n", error, __LINE__);
		exit(EXIT_FAILURE);
	}

	error = cudaMalloc((void **) &d_C, mem_size_C);

	if (error != cudaSuccess)
	{
		printf("cudaMalloc d_C returned error code %d, line(%d)\n", error, __LINE__);
		exit(EXIT_FAILURE);
	}

	// copy host memory to device
	error = cudaMemcpy(d_A, h_A, mem_size_A, cudaMemcpyHostToDevice);

	if (error != cudaSuccess)
	{
		printf("cudaMemcpy (d_A,h_A) returned error code %d, line(%d)\n", error, __LINE__);
		exit(EXIT_FAILURE);
	}

	error = cudaMemcpy(d_B, h_B, mem_size_B, cudaMemcpyHostToDevice);

	if (error != cudaSuccess)
	{
		printf("cudaMemcpy (d_B,h_B) returned error code %d, line(%d)\n", error, __LINE__);
		exit(EXIT_FAILURE);
	}

	// Setup execution parameters
	dim3 threads(block_size, block_size);
	dim3 grid(width / threads.x, width / threads.y);

	cudaDeviceSynchronize();

	// Allocate CUDA events that we'll use for timing
	cudaEvent_t start;
	error = cudaEventCreate(&start);

	if (error != cudaSuccess)
	{
		fprintf(stderr, "Failed to create start event (error code %s)!\n", cudaGetErrorString(error));
		exit(EXIT_FAILURE);
	}

	cudaEvent_t stop;
	error = cudaEventCreate(&stop);

	if (error != cudaSuccess)
	{
		fprintf(stderr, "Failed to create stop event (error code %s)!\n", cudaGetErrorString(error));
		exit(EXIT_FAILURE);
	}

	// Record the start event
	error = cudaEventRecord(start, NULL);

	if (error != cudaSuccess)
	{
		fprintf(stderr, "Failed to record start event (error code %s)!\n", cudaGetErrorString(error));
		exit(EXIT_FAILURE);
	}

	// Execute the kernel
	int nIter = 300;

	for (int j = 0; j < nIter; j++)
	{
		switch(v) {
			case 1:
				grid.x = 1; grid.y = 1; grid.z = 1;
				matrixMulv1<16> <<< grid, threads >>>(d_C, d_A, d_B, width);
				break;
			case 2: 
				matrixMulv2<16> <<< grid, threads >>>(d_C, d_A, d_B, width);
				break;
			case 3: 
				matrixMulv3<16> <<< grid, threads >>>(d_C, d_A, d_B, width);
				break;
			case 4: 
				matrixMulv4<16> <<< grid, threads >>>(d_C, d_A, d_B, width);
				break;
			case 5: 
				//matrixMulv5<16> <<< grid, threads >>>(d_C, d_A, d_B, width);
				break;
		}
	}

	// Record the stop event
	error = cudaEventRecord(stop, NULL);

	if (error != cudaSuccess)
	{
		fprintf(stderr, "Failed to record stop event (error code %s)!\n", cudaGetErrorString(error));
		exit(EXIT_FAILURE);
	}

	// Wait for the stop event to complete
	error = cudaEventSynchronize(stop);

	if (error != cudaSuccess)
	{
		fprintf(stderr, "Failed to synchronize on the stop event (error code %s)!\n", cudaGetErrorString(error));
		exit(EXIT_FAILURE);
	}

	float msecTotal = 0.0f;
	error = cudaEventElapsedTime(&msecTotal, start, stop);

	if (error != cudaSuccess)
	{
		fprintf(stderr, "Failed to get time elapsed between events (error code %s)!\n", cudaGetErrorString(error));
		exit(EXIT_FAILURE);
	}

	// Compute and print the performance
	float msecPerMatrixMul = msecTotal / nIter;
	double flopsPerMatrixMul = 2.0 * (double)width * (double)width * (double)width;
	double gigaFlops = (flopsPerMatrixMul * 1.0e-9f) / (msecPerMatrixMul / 1000.0f);
	printf(
			"Performance= %.2f GFlop/s, Time= %.3f msec, Size= %.0f Ops, WorkgroupSize= %u threads/block\n",
			gigaFlops,
			msecPerMatrixMul,
			flopsPerMatrixMul,
			threads.x * threads.y);

	// Copy result from device to host
	error = cudaMemcpy(h_C, d_C, mem_size_C, cudaMemcpyDeviceToHost);

	if (error != cudaSuccess)
	{
		printf("cudaMemcpy (h_C,d_C) returned error code %d, line(%d)\n", error, __LINE__);
		exit(EXIT_FAILURE);
	}

	printf("Checking computed result for correctness: ");
	bool correct = true;

	for (int i = 0; i < (int)(dimsC.x * dimsC.y); i++)
	{
		if (fabs(h_C[i] - (width * valB)) > 1e-5)
		{
			printf("Error! Matrix[%05d]=%.8f, ref=%.8f error term is > 1e-5\n", i, h_C[i], width*valB);
			correct = false;
		}
	}

	printf("%s\n", correct ? "OK" : "FAIL");

	// Clean up memory
	free(h_A);
	free(h_B);
	free(h_C);
	cudaFree(d_A);
	cudaFree(d_B);
	cudaFree(d_C);

	printf("\nNote: For peak performance, please refer to the matrixMulCUBLAS example.\n");

	cudaDeviceReset();

	if (correct)
	{
		return EXIT_SUCCESS;
	}
	else
	{
		return EXIT_FAILURE;
	}
}


/**
 * Program main
 */
int main(int argc, char **argv)
{
	printf("[Matrix Multiply Using CUDA] - Starting...\n");

	if (checkCmdLineFlag(argc, (const char **)argv, "help") ||
			checkCmdLineFlag(argc, (const char **)argv, "?"))
	{
		printf("Usage -device=n (n >= 0 for deviceID)\n");
		printf("      -wA=WidthA -hA=HeightA (Width x Height of Matrix A)\n");
		printf("      -wB=WidthB -hB=HeightB (Width x Height of Matrix B)\n");
		printf("  Note: Outer matrix dimensions of A & B matrices must be equal.\n");

		exit(EXIT_SUCCESS);
	}

	// By default, we use device 0, otherwise we override the device ID based on what is provided at the command line
	int devID = 0;

	if (checkCmdLineFlag(argc, (const char **)argv, "device"))
	{
		devID = getCmdLineArgumentInt(argc, (const char **)argv, "device");
		cudaSetDevice(devID);
	}

	cudaError_t error;
	cudaDeviceProp deviceProp;
	error = cudaGetDevice(&devID);

	if (error != cudaSuccess)
	{
		printf("cudaGetDevice returned error code %d, line(%d)\n", error, __LINE__);
	}

	error = cudaGetDeviceProperties(&deviceProp, devID);

	if (deviceProp.computeMode == cudaComputeModeProhibited)
	{
		fprintf(stderr, "Error: device is running in <Compute Mode Prohibited>, no threads can use ::cudaSetDevice().\n");
		exit(EXIT_SUCCESS);
	}

	if (error != cudaSuccess)
	{
		printf("cudaGetDeviceProperties returned error code %d, line(%d)\n", error, __LINE__);
	}
	else
	{
		printf("GPU Device %d: \"%s\" with compute capability %d.%d\n\n", devID, deviceProp.name, deviceProp.major, deviceProp.minor);
	}

	// Use a larger block size for Fermi and above
	int block_size = 16;
	int size=160;
	int V;
	// width of Matrix A
	if (checkCmdLineFlag(argc, (const char **)argv, "size"))
	{
		size = getCmdLineArgumentInt(argc, (const char **)argv, "size");
	}
	if (checkCmdLineFlag(argc, (const char **)argv, "V"))
	{
		V = getCmdLineArgumentInt(argc, (const char **)argv, "V");
	}



	int matrix_result = matrixMultiply(argc, argv, block_size, size, V);

	exit(matrix_result);
}
