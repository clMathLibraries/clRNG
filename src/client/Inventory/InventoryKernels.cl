
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

#define CLRNG_ENABLE_SUBSTREAMS
/*! [clRNG header] */
#include <clRNG/mrg31k3p.clh>
/*! [clRNG header] */

__constant int L = 100;
__constant int c = 2;
__constant double h = 0.1;
__constant int K = 10;
__constant int k = 1;
__constant double p = 0.95;


#pragma OPENCL EXTENSION cl_amd_printf : enable
#pragma OPENCL EXTENSION cl_amd_fp64 : enable

//Important : all the variables that start with the prefix 'param_' are passed to the Open CL C compiler as inline parameter (see.: BuildOptions)

//----------------------------------------------------------------------------

double inventorySimulateOneRun(int m, int s, int S, clrngMrg31k3pStream* stream_demand, clrngMrg31k3pStream* stream_order)
{
	int Xj = S, Yj;                // Stock in the morning and in the evening.
	double profit = 0.0;           // Cumulated profit.
	for (int j = 0; j < m; j++) {
		// Generate and subtract the demand for the day.
		Yj = Xj - clrngMrg31k3pRandomInteger(stream_demand, 0, L);
		if (Yj < 0)
			Yj = 0;                  // Lost demand.
		profit += c * (Xj - Yj) - h * Yj;
		if ((Yj < s) && (clrngMrg31k3pRandomU01(stream_order) < p)) {
			// We have a successful order.
			profit -= K + k * (S - Yj);
			Xj = S;
		}
		else
			Xj = Yj;
	}

	return profit / m;


}

//************************************************************************
// One policy
//************************************************************************

//Case (a) and (b) : Simulate n runs on n work items using two streams and their substreams or 2*n streams
__kernel void inventorySimulateGPU(__global clrngMrg31k3pHostStream* streams_demand,__global clrngMrg31k3pHostStream* streams_order,__global double* stat_profit)
{
	// Each of the n work items executes the following code.
	int gid = get_global_id(0); // Id of this work item.

	// Make local copies of the stream states in private memory.
	clrngMrg31k3pStream stream_demand_d, stream_order_d;
	clrngMrg31k3pCopyOverStreamsFromGlobal(1, &stream_demand_d, &streams_demand[gid]); 
	clrngMrg31k3pCopyOverStreamsFromGlobal(1, &stream_order_d, &streams_order[gid]);

	stat_profit[gid] = inventorySimulateOneRun(param_m, param_s, param_S, &stream_demand_d, &stream_order_d);
}

//Case (c) : use distinct streams across the work items and n2 substreams within each work item.
__kernel void inventorySimulSubstreamsGPU(__global clrngMrg31k3pHostStream *streams_demand,	__global clrngMrg31k3pHostStream *streams_order,__global double *stat_profit) 
{
	// Each of the n1 work items executes the following to simulate n2 runs.
	int gid = get_global_id(0); // Id of this work item
	int n1 = get_global_size(0); //Total number of work items

	// Make local copies of the stream states in private memory
	clrngMrg31k3pStream stream_demand_d, stream_order_d;
	clrngMrg31k3pCopyOverStreamsFromGlobal(1, &stream_demand_d, &streams_demand[gid]); 
	clrngMrg31k3pCopyOverStreamsFromGlobal(1, &stream_order_d, &streams_order[gid]);

	for (int i = 0; i < param_n2; i++) {
		stat_profit[i * n1 + gid] = inventorySimulateOneRun(param_m, param_s, param_S, &stream_demand_d, &stream_order_d);
		clrngMrg31k3pForwardToNextSubstreams(1, &stream_demand_d);
		clrngMrg31k3pForwardToNextSubstreams(1, &stream_order_d);
	}
}

//Case (d) : each work item uses 2*n2 distinct streams instead of using substreams.
__kernel void inventorySimul_DistinctStreams_GPU(__global clrngMrg31k3pHostStream *streams_demand,	__global clrngMrg31k3pHostStream *streams_order,__global double *stat_profit) 
{
	// Each of the n1 work items executes the following to simulate n2 runs.
	int gid = get_global_id(0); // Id of this work item.
	int n1 = get_global_size(0); //Total number of work items

	clrngMrg31k3pStream stream_demand_d, stream_order_d;

	for (int i = 0; i < param_n2; i++) {
		// Make local copies of the stream states, in private memory.
		clrngMrg31k3pCopyOverStreamsFromGlobal(1, &stream_demand_d, &streams_demand[i * n1 + gid]);
		clrngMrg31k3pCopyOverStreamsFromGlobal(1, &stream_order_d, &streams_order[i * n1 + gid]);

		stat_profit[i * n1 + gid] = inventorySimulateOneRun(param_m, param_s, param_S, &stream_demand_d, &stream_order_d);
	}
}

//************************************************************************
// Several policies
//************************************************************************

//Simulate n2 runs on n1p workitmes using n1 streams and their substreams
__kernel void inventorySimulPoliciesGPU(__global clrngMrg31k3pHostStream *streams_demand, __global clrngMrg31k3pHostStream *streams_order, __global double *stat_profit
	                                   ,__global cl_int *s, __global cl_int *S) 
{
	// Each of the n1*P work items executes the following to simulate n2 runs.
	int gid = get_global_id(0);    // Id of this work item.
	int n1p = get_global_size(0);  // n1*P = Total number of work items
	int n1  = n1p / param_P;       // Number of streams.
	int k   = gid / n1;            // Index that identify which policy the work item will use. in case p = 2 : k = 0 or k = 1
	int j   = gid % n1;            // Index that identify the index of the work item modulo n1, in case n1=100 : j=0..99

	// Make local copies of the stream states, in private memory.
	clrngMrg31k3pStream stream_demand_d, stream_order_d;
	clrngMrg31k3pCopyOverStreamsFromGlobal(1, &stream_demand_d, &streams_demand[j]);
	clrngMrg31k3pCopyOverStreamsFromGlobal(1, &stream_order_d, &streams_order[j]);

	for (int i = 0; i < param_n2; i++) 
	{
		stat_profit[i * n1p + gid] = inventorySimulateOneRun(param_m, s[k], S[k], &stream_demand_d, &stream_order_d);
		clrngMrg31k3pForwardToNextSubstreams(1, &stream_demand_d);
		clrngMrg31k3pForwardToNextSubstreams(1, &stream_order_d);
	}
}

