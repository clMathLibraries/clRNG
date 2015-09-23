
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
#include <clRNG/clRNG.h>
#include <clRNG/mrg31k3p.h>
#include "../common.h"
#include "Types.h"


//#define NVIDIA
/* [constants] */
#define L	100
#define c	2
#define h	0.1
#define K	10
#define k	1
#define per	0.95
/* [constants] */

static char _options[255];

//************************************************************************
// Device Simulation
//************************************************************************

char * Build_Options(char _options[255], int m, int n2, int s, int S, int P)
{
	char _m[7], _n2[7], _s[7], _S[7], _p[7];
	sprintf(_m, "%d", m);
	sprintf(_n2,"%d", n2);
	sprintf(_s, "%d", s);
	sprintf(_S, "%d", S);
	sprintf(_p, "%d", P);

	strcpy(_options, " -Dparam_m=\"");
	strcat(_options, _m);
	strcat(_options, "\"");

	strcat(_options, " -Dparam_n2=\"");
	strcat(_options, _n2);
	strcat(_options, "\"");

	strcat(_options, " -Dparam_s=\"");
	strcat(_options, _s);
	strcat(_options, "\"");

	strcat(_options, " -Dparam_S=\"");
	strcat(_options, _S);
	strcat(_options, "\"");

	strcat(_options, " -Dparam_P=\"");
	strcat(_options, _p);

	return strcat(_options, "\"");
}
clrngStatus inventorySimulateRunsGPU(cl_context context, cl_device_id device, cl_command_queue queue, 
	                            int m, int* s, int* S, int P, int n, int n1, int n2, ExecOption Option,
								const char * kernelName, size_t streamsBufSize, clrngMrg31k3pStream * streams_demand, clrngMrg31k3pStream * streams_order, 
								double *stat, simResult * results)
{
	cl_int err;
	size_t global_size;

	// Total number of work-items.
	if (n1 == 0) global_size = n;
	else{
		global_size = n1;
	}

	// Buffers to store the streams and profits.
	cl_mem streams_demand_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, streamsBufSize, streams_demand, &err);
	check_error(err, "%s(): cannot create streams demand buffer", __func__);

	cl_mem streams_order_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, streamsBufSize, streams_order, &err);
	check_error(err, "%s(): cannot create streams order buffer", __func__);

	cl_mem stat_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, n * sizeof(double), NULL, &err);
	check_error(err, "%s(): cannot create stat buffer", __func__);

	//Buffers to store policies
	cl_mem s_buffer = NULL, S_buffer = NULL;
	if (Option == Option2)
	{		
		s_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, P * sizeof(int), s, &err);
		check_error(err, "%s(): cannot create streams demand buffer", __func__);

		S_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, P * sizeof(int), S, &err);
		check_error(err, "%s(): cannot create streams order buffer", __func__);
	}

	// Create kernel, that executes inventorySimulOne for each policy, on each work item.
	cl_program program = build_program_from_file(context, device,
		"client/Inventory/InventoryKernels.cl", PATH_RELATIVE_TO_LIB,
		Build_Options(_options, m, n2, s[0], S[0], P));

	cl_kernel kernel = clCreateKernel(program, kernelName, &err);
	check_error(err, "%s(): cannot create Kernel", __func__);

	// Set arguments for kernel and enqueue that kernel.
	err |= clSetKernelArg(kernel, 0, sizeof(streams_demand_buffer), &streams_demand_buffer);
	err |= clSetKernelArg(kernel, 1, sizeof(streams_order_buffer), &streams_order_buffer);
	err |= clSetKernelArg(kernel, 2, sizeof(stat_buffer), &stat_buffer);
	
	if (Option == Option2)
	{
		err |= clSetKernelArg(kernel, 3, sizeof(s_buffer), &s_buffer);
		err |= clSetKernelArg(kernel, 4, sizeof(S_buffer), &S_buffer);
	}
	
	check_error(err, "%s(): cannot create Kernel arguments", __func__);


	cl_event prof_event;        // Used for timing execution.
	
	// Enqueue kernels
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, &prof_event);
	check_error(err, "%s(): cannot enqueue Kernel", __func__);

	// Finish processing the queue and compute the total time for executing the program.
	clFinish(queue);
	
	cl_ulong time_start, time_end;
	clGetEventProfilingInfo(prof_event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
	clGetEventProfilingInfo(prof_event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
	cl_ulong total_GPU_time = time_end - time_start;

	// After execution, transfer the results in the stat_diff array.
	err = clEnqueueReadBuffer(queue, stat_buffer, CL_TRUE, 0, n * sizeof(double), stat, 0, NULL, NULL);
	check_error(err, "%s(): cannot read the buffer", __func__);

	if (Option != Option2) printf("\nTotal GPU time (sec.): %1.5f\n", (total_GPU_time / 1.0e9));

	computeCI(n, stat, results);

	//Record results
	if (results != NULL){
		results->SimType = GPU_Exec;
		results->CPU_time = (total_GPU_time / 1.0e9);
	}

	//printf("***************************************************************************\n");

	// Deallocate resources
	clReleaseEvent(prof_event);
	clReleaseMemObject(stat_buffer);
	clReleaseMemObject(streams_demand_buffer);
	clReleaseMemObject(streams_order_buffer);
	clReleaseMemObject(s_buffer);
	clReleaseMemObject(S_buffer);
	clReleaseKernel(kernel);
	clReleaseProgram(program);

	return (clrngStatus)EXIT_SUCCESS;
}

//************************************************************************
// CPU Simulation
//************************************************************************
/* [simulate one run] */
double inventorySimulateOneRun(int m, int s, int S, clrngMrg31k3pStream* stream_demand, clrngMrg31k3pStream*  stream_order)
{
	int Xj = S, Yj;                // Stock in the morning and in the evening.
	double profit = 0.0;           // Cumulated profit.
	for (int j = 0; j < m; j++) {
		// Generate and subtract the demand for the day.
		Yj = Xj - clrngMrg31k3pRandomInteger(stream_demand, 0, L);
		if (Yj < 0)
			Yj = 0;                  // Lost demand.
		profit += c * (Xj - Yj) - h * Yj;
		if ((Yj < s) && (clrngMrg31k3pRandomU01(stream_order) < per)) {
			// We have a successful order.
			profit -= K + k * (S - Yj);
			Xj = S;
		}
		else
			Xj = Yj;
	}
	
	return profit / m;
}
/* [simulate one run] */

clrngStatus inventorySimulateRunsCPU(int m, int s, int S, int n, clrngMrg31k3pStream* stream_demand, clrngMrg31k3pStream* stream_order, 
	                                 double *stat_profit, ExecType execType, simResult * results)
{
	clock_t start = clock();

	if (execType == basic){
		// (basic case) : Performs n independent simulation runs of the system for m days with the (s,S) policy
		// using a single stream and the same substream for everything, and saves daily profit values.
		// Equivalent implementation of inventorySimulateRunsOneStream() from the document.
		for (int i = 0; i < n; i++)
			stat_profit[i] = inventorySimulateOneRun(m, s, S, stream_demand, stream_demand);
		if (results != NULL) results->ExecOption = 1;
	}

	else if (execType == Case_a){
		//Case (a) : Similar to (basic), but using two streams and their substreams.
		// Equivalent implementation of inventorySimulateRunsSubstreams() from the document.
		for (int i = 0; i < n; i++){
			stat_profit[i] = inventorySimulateOneRun(m, s, S, stream_demand, stream_order);
			clrngMrg31k3pForwardToNextSubstreams(1, stream_demand);
			clrngMrg31k3pForwardToNextSubstreams(1, stream_order);
		}
		if (results != NULL) results->ExecOption = 2;
	}

	else if (execType == Case_b){
		//Case (b) : Similar to (basic), but with two arrays of n streams each, using a new pair of streams for each run.
		// Equivalent implementation of inventorySimulateRunsManyStreams() from the document.
		for (int i = 0; i < n; i++)
			stat_profit[i] = inventorySimulateOneRun(m, s, S, &stream_demand[i], &stream_order[i]);
		if (results != NULL) results->ExecOption = 3;
	}
	//Compute Execution Time
	clock_t end = clock();
	float CPU_time = (float)(end - start) / CLOCKS_PER_SEC;
	printf("\nTotal CPU time (sec.): %1.6f\n", CPU_time);

	//Compute CI
	computeCI(n, stat_profit, results);

	//Record results
	if (results != NULL){
		results->SimType = CPU_Exec;
		results->CPU_time = CPU_time;
	}

	return (clrngStatus)EXIT_SUCCESS;
}
