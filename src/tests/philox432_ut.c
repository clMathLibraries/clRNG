
 /*
 ***********************************************************************
 Copyright (c) 2015 Advanced Micro Devices, Inc. 
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions 
 are met:
 
 1. Redistributions of source code must retain the above copyright 
 notice, this list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright 
 notice, this list of conditions and the following disclaimer in the 
 documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 ***********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "../include/clRNG/private/Random123/philox.h"
#include "uniform.hpp"


#include <clRNG/philox432.h>
#include <clRNG/lfsr113.h>
#include <clRNG/mrg31k3p.h>
#include <clRNG/mrg32k3a.h>

typedef r123::Philox4x32 CBRNG;

struct clrngPhilox432StreamCreator_ {
	clrngPhilox432StreamState clrngPhilox432NextState;
	clrngPhilox432Counter JumpDistance;
};


double runRandom123(CBRNG g, CBRNG::ctr_type  ctr, CBRNG::key_type key, CBRNG::ctr_type rand, int n)
{
	double sum = 0.0;
	int j = 0;
	for (int i = 0; i < n; i++)
	{
		if (i % 4 == 0)
		{
			ctr[0] = j;
			rand = g(ctr, key);
			sum += r123::u01<double>(rand[i % 4]);

			j += 1;
		}
		else sum += r123::u01<double>(rand[i % 4]);
	}
	return sum;
}

void FillBufferOnGPU(void * streams,
	size_t streamBufferSize,
	size_t streamCount,
	size_t numberCount,
	clrngStatus(*clrngDeviceRandomU01Array)(size_t, cl_mem, size_t, cl_mem, cl_uint, cl_command_queue*, cl_uint, const cl_event*, cl_event*, cl_bool))
{
	cl_int err;

	//Platform
	cl_platform_id platforms[5];
	cl_uint num_platforms;

	err = clGetPlatformIDs(5, platforms, &num_platforms);
	cl_platform_id platform = platforms[0];

	//Devices
	cl_uint num_devices;
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);

	cl_device_id* devices = (cl_device_id*)malloc(num_devices * sizeof(cl_device_id));
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);

	//Context
	cl_context context = clCreateContext(NULL, num_devices, devices, NULL, NULL, &err);

	//commQueue
#ifdef CL_VERSION_2_0
	cl_queue_properties queue_properties[] = {
		CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE,
		0 };
	cl_command_queue commQueues = clCreateCommandQueueWithProperties(context, devices[0], queue_properties, &err);
#else
	cl_command_queue commQueues = clCreateCommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE, &err);
#endif

	//Create Number buffer
	cl_mem outBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, numberCount * sizeof(double), NULL, &err);

	//Profiling Events
	cl_event outEvents = NULL;


	//streams buffer
	cl_mem streams_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, streamBufferSize, streams, &err);

	//*************************************************************************************************************************
	//Call Kernel

	clrngDeviceRandomU01Array(streamCount, streams_buffer, numberCount, outBuffer, 1, &commQueues, 0, NULL, &outEvents, false);

	clWaitForEvents(1, &outEvents);

	//Print Total GPU Time
	cl_ulong time_start, time_end, total_GPU_time;
	clGetEventProfilingInfo(outEvents, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
	clGetEventProfilingInfo(outEvents, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
	total_GPU_time = time_end - time_start;
	printf(" Total GPU time (sec.) %1.7f\n\n", (total_GPU_time / 1.0e9));

	//Release resources
	clReleaseEvent(outEvents);
	clReleaseMemObject(streams_buffer);
	err = clReleaseCommandQueue(commQueues);

	for (size_t i = 0; i < num_devices; i++)
		clReleaseDevice(devices[i]);
	free(devices);

	clReleaseContext(context);
	free(streams);

}
bool is_array_eq(cl_uint *x, cl_uint *y, size_t n)
{
	for (size_t i = 0; i < n; i++)
		if ((x[i] - y[i]) > 0)
			return false;
	return true;
}

bool is_counter_eq(clrngPhilox432Counter x, clrngPhilox432Counter y)
{
	if ((x.H.msb == y.H.msb) &&
		(x.H.lsb == y.H.lsb) &&
		(x.L.msb == y.L.msb) &&
		(x.L.lsb == y.L.lsb))
		return true;
	return false;
}

void printAfterAdvance(cl_uint count, clrngPhilox432Stream* streams, cl_int e, cl_int c)
{
	clrngPhilox432AdvanceStreams(count, streams, e, c);

	printf("\n (e,c)=(%d,%d) = %.0f states: \t", e, c, (e >=0 ? 1 : -1)*(e == 0 ? 0 : powf(2, abs(e))) + c); clrngPhilox432WriteStreamInfo(streams, stdout);

}
int main()
{
	clrngPhilox432Stream* stream = NULL;

	clrngStatus err;
	size_t streamBufferSize;
	size_t NbrStreams = 1;

	int option = 9;

	//Testing that the counter is correctly advancing.
	if (option == 1){

		clrngPhilox432StreamCreator* creator = clrngPhilox432CopyStreamCreator(NULL, &err);

		clrngPhilox432StreamState * baseState = (clrngPhilox432StreamState *)malloc(sizeof(clrngPhilox432StreamState));
		*baseState = { { { 0xFFFFFFFF, 0xFFFFFFFF }, { 0xFFFFFFFF, 0xFFFFFFFA } },
		{ 0, 0, 0, 0 },
		4 };

		clrngPhilox432SetBaseCreatorState(creator, baseState);

		stream = clrngPhilox432CreateStreams(creator, NbrStreams, &streamBufferSize, &err);

		for (int i = 0; i < 36; i++)
		{
			printf("U : %16.15f \t states : ", clrngPhilox432RandomU01(stream));
			clrngPhilox432WriteStreamInfo(stream, stdout);
			if ((i + 1) % 4 == 0) printf("\n");
		}

		//Assert
		clrngPhilox432Counter  expectedValue = { { 0, 0 }, { 0, 3 } };
		assert(is_counter_eq(stream->current.ctr, expectedValue));

		printf("UTest - Progress in clrngPhilox432RandomU01 : Passed!\n");
	}

	//Testing Change Streams Spacing
	else 	if (option == 2){

		clrngPhilox432StreamCreator* creator = clrngPhilox432CopyStreamCreator(NULL, &err);
		clrngPhilox432Counter  expectedValue = { { 0, 16 }, { 0, 0 } };
		assert(is_counter_eq(creator->JumpDistance, expectedValue));

		clrngPhilox432ChangeStreamsSpacing(creator, 64, 0);
		expectedValue = { { 0, 1 }, { 0, 0 } };
		assert(is_counter_eq(creator->JumpDistance, expectedValue));

		stream = clrngPhilox432CreateStreams(creator, 1, &streamBufferSize, &err);
		expectedValue = { { 0, 1 }, { 0, 0 } };
		assert(is_counter_eq(creator->clrngPhilox432NextState.ctr, expectedValue));

		stream = clrngPhilox432CreateStreams(creator, 1, &streamBufferSize, &err);
		expectedValue = { { 0, 2 }, { 0, 0 } };
		assert(is_counter_eq(creator->clrngPhilox432NextState.ctr, expectedValue));

		printf("UTest - clrngPhilox432ChangeStreamsSpacing : Passed!\n");
	}

	//Comparing the stream states between clrngPhilox432 and Random123
	else if (option == 3)
	{
		stream = clrngPhilox432CreateStreams(NULL, NbrStreams, &streamBufferSize, &err);

		//Random123 vars
		typedef r123::Philox4x32 CBRNG;
		CBRNG g;
		CBRNG::ctr_type  ctr = { { 0, 0, 0, 0 } };
		CBRNG::key_type key = { { 0xCD9E8D57, 0xD2511F53 } };
		
		int j = 0;
		for (int i = 0; i < 12; i++)
		{
			//Print out Randon123 counter states
			if (i % 4 == 0)
			{
				ctr[0] = j;
				CBRNG::ctr_type rand = g(ctr, key);

				printf("\nPackage = { %u %u %u %u} \t Rand = { %u %u %u %u} \n", ctr[3], ctr[2], ctr[1], ctr[0], rand[3], rand[2], rand[1], rand[0]);
				j += 1;
			}

			//Print out clrngPhilox432 counter state
			clrngPhilox432RandomU01(stream);
			clrngPhilox432WriteStreamInfo(stream, stdout);
		}
	}

	//Comparing the u01 between clrngPhilox432 and Random123(uniform.hpp)
	else if (option == 4)
	{
		stream = clrngPhilox432CreateStreams(NULL, NbrStreams, &streamBufferSize, &err);

		//Random123 variables
		typedef r123::Philox4x32 CBRNG;
		CBRNG g;
		CBRNG::ctr_type  ctr = { { 0, 0, 0, 0 } };
		CBRNG::key_type key = { { 0xdeadbeef, 0xbadcafe } };
		CBRNG::ctr_type rand;

		int j = 0;
		for (int i = 0; i < 120; i++)
		{
			//Generate u01 from Random123
			if (i % 4 == 0)
			{
				printf("\n");
				ctr[0] = j;
				rand = g(ctr, key);

				printf("Random123\t %16.15f\t", r123::u01<double>(rand[i % 4]));

				j += 1;
			}
			else printf("Random123\t %16.15f\t", r123::u01<double>(rand[i % 4]));

			//generate u01 from clrngPhilox
			printf("clrngPhilox\t %16.15f\n", clrngPhilox432RandomU01(stream));
		}

	}

	//comparimng execution time between clrngPhilox432 and Random123
	else if (option == 5)
	{
		double sum = 0.0;
		int n = 100000000;
		stream = clrngPhilox432CreateStreams(NULL, NbrStreams, &streamBufferSize, &err);

		//Random123 variables

		CBRNG g;
		CBRNG::ctr_type  ctr = { { 0, 0, 0, 0 } };
		CBRNG::key_type key = { { 0xdeadbeef, 0xbadcafe } };
		CBRNG::ctr_type rand = {};

		//Random123
		clock_t start = clock();
		runRandom123(g, ctr, key, rand, n);
		clock_t end = clock();
		float CPU_time = (float)(end - start) / CLOCKS_PER_SEC;

		printf("\nTotal CPU time (sec.) for Random123 : %1.6f\n", CPU_time);


		//clrngPhilox
		start = clock();
		for (int i = 0; i < n; i++)
		{
			sum += clrngPhilox432RandomU01(stream);
			//printf("clrngPhilox\t %16.15f\n", u);
		}
		//Compute Execution Time
		end = clock();
		CPU_time = (float)(end - start) / CLOCKS_PER_SEC;
		printf("\nTotal CPU time (sec.) for clRngPhilox432 : %7.6f\n", CPU_time);
	}

	//comparing execution time of the four rngs on CPU 
	else if (option == 6)
	{
		double u;
		int n = 100000000;

		//clRNG
		clrngPhilox432Stream * stream = clrngPhilox432CreateStreams(NULL, NbrStreams, &streamBufferSize, &err);
		clrngLfsr113Stream* stream2 = clrngLfsr113CreateStreams(NULL, NbrStreams, &streamBufferSize, &err);
		clrngMrg31k3pStream* stream3 = clrngMrg31k3pCreateStreams(NULL, NbrStreams, &streamBufferSize, &err);
		clrngMrg32k3aStream* stream4 = clrngMrg32k3aCreateStreams(NULL, NbrStreams, &streamBufferSize, &err);

		//clrngPhilox
		clock_t start = clock();
		for (int i = 0; i < n; i++)
		{
			u = clrngPhilox432RandomU01(stream);
			//printf("clrngPhilox\t %16.15f\n", u);
		}
		clock_t end = clock();
		float CPU_time = (float)(end - start) / CLOCKS_PER_SEC;
		printf("\nTotal CPU time (sec.) for clRngPhilox432 : %1.6f\n", CPU_time);

		//clrngLfsr113
		start = clock();
		for (int i = 0; i < n; i++)
		{
			u = clrngLfsr113RandomU01(stream2);
			//printf("clrngPhilox\t %16.15f\n", u);
		}
		end = clock();
		CPU_time = (float)(end - start) / CLOCKS_PER_SEC;
		printf("\nTotal CPU time (sec.) for clRngLfsr113 : %1.6f\n", CPU_time);

		//clrngMrg31k3p
		start = clock();
		for (int i = 0; i < n; i++)
		{
			u = clrngMrg31k3pRandomU01(stream3);
			//printf("clrngPhilox\t %16.15f\n", u);
		}
		end = clock();
		CPU_time = (float)(end - start) / CLOCKS_PER_SEC;
		printf("\nTotal CPU time (sec.) for clRngMrg31k3p : %1.6f\n", CPU_time);

		//clrngMrg32k3a
		start = clock();
		for (int i = 0; i < n; i++)
		{
			u = clrngMrg32k3aRandomU01(stream4);
			//printf("clrngPhilox\t %16.15f\n", u);
		}
		end = clock();
		CPU_time = (float)(end - start) / CLOCKS_PER_SEC;
		printf("\nTotal CPU time (sec.) for clRngMrg32k3a : %1.6f\n", CPU_time);
	}

	//Comparing execution time on GPU for the rngs
	else if (option == 7)
	{
		size_t streamCount = 100;
		size_t numberCount = 10000000;
		size_t streamBufferSize;
		//*************************************************************************************************************************
		printf("DeviceRandomU01Array with Philox432 :");
		clrngPhilox432Stream * streams = clrngPhilox432CreateStreams(NULL, streamCount, &streamBufferSize, NULL);
		FillBufferOnGPU(streams, streamBufferSize, streamCount, numberCount, clrngPhilox432DeviceRandomU01Array_);


		//*************************************************************************************************************************
		printf("DeviceRandomU01Array with Lfsr113 :");
		clrngLfsr113Stream * streams2 = clrngLfsr113CreateStreams(NULL, streamCount, &streamBufferSize, NULL);
		FillBufferOnGPU(streams2, streamBufferSize, streamCount, numberCount, clrngLfsr113DeviceRandomU01Array_);

		//*************************************************************************************************************************
		printf("DeviceRandomU01Array with Mrg31k3p :");
		clrngMrg31k3pStream * streams3 = clrngMrg31k3pCreateStreams(NULL, streamCount, &streamBufferSize, NULL);
		FillBufferOnGPU(streams3, streamBufferSize, streamCount, numberCount, clrngMrg31k3pDeviceRandomU01Array_);

		//*************************************************************************************************************************
		printf("DeviceRandomU01Array with Mrg32k3a :");
		clrngMrg32k3aStream * streams4 = clrngMrg32k3aCreateStreams(NULL, streamCount, &streamBufferSize, NULL);
		FillBufferOnGPU(streams4, streamBufferSize, streamCount, numberCount, clrngMrg32k3aDeviceRandomU01Array_);

	}

	//Testing clrngPhilox432AdvanceStreams
	else if (option == 8)
	{
		stream = clrngPhilox432CreateStreams(NULL, NbrStreams, &streamBufferSize, &err);
		printf("Initial state : \t\t"); clrngPhilox432WriteStreamInfo(stream, stdout);

		/*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 2, 3);
		clrngPhilox432RandomU01(stream);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 1, 1);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 1, 2);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 2, 3);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 2, 4);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 2, 5);

		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 1, -1);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 1, -2);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 2, -3);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 2, -4);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 2, -5); 
		//printAfterAdvance(1, stream, -5, -2);
		//printf("\n\n");
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, 0, -2);

		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, -1, -1);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, -1, -2);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, -2, -3);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, -2, -4);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, -2, -5);
		//
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, -1, 1);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, -1, 2);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, -2, 3);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, -2, 4);
		///*clrngPhilox432RewindStreams(1, stream);*/ printAfterAdvance(1, stream, -2, 5); 
		//printAfterAdvance(1, stream, 5, 2);
	}
	else if (option == 9)
	{
		// créer deux copies identiques du même stream
		clrngPhilox432Stream* stream1 = clrngPhilox432CreateStreams(NULL, 1, NULL, NULL);
		clrngPhilox432Stream* stream2 = clrngPhilox432CopyStreams(1, stream1, NULL);

		// comparer les états initiaux
		//printf("INITIAL STATES\n\n");
		clrngPhilox432WriteStreamInfo(stream1, stdout);
		clrngPhilox432WriteStreamInfo(stream2, stdout);

		// avancer les deux streams du même nombre d'états
		size_t step_size = 8;
		clrngPhilox432AdvanceStreams(1, stream1, 0, step_size);

		for (size_t i = 0; i < step_size; i++)
		{
			clrngPhilox432RandomU01(stream2);
			//clrngPhilox432WriteStreamInfo(stream2, stdout);
		}

		// comparer les états finaux
		printf("FINAL STATES\n\n");
		clrngPhilox432WriteStreamInfo(stream1, stdout);
		clrngPhilox432WriteStreamInfo(stream2, stdout);

		// comparer les prochaines valeurs
		printf("NEXT VALUES\n\n");
		for (size_t i = 0; i < 5; i++)
			printf("%.12f\n%.12f\n\n",
			clrngPhilox432RandomU01(stream1),
			clrngPhilox432RandomU01(stream2));

		// libérer la mémoire
		clrngPhilox432DestroyStreams(stream1);
		clrngPhilox432DestroyStreams(stream2);
	}
	//verify with the inventory example by generating Profit Matrix : done!

	getchar();
	return 0;
}
