
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
#include <clRNG/clRNG.h>
#include "../common.h"
#include "Policies.h"
#include "SimulateRuns.h"

/*! [clRNG header] */
#include <clRNG/mrg31k3p.h>
/*! [clRNG header] */

//************************************************************************
// Policies
//************************************************************************
int one_Policy(cl_context context, cl_device_id device, cl_command_queue queue, void* data_)
{
	OnePolicyData* data = (OnePolicyData*)data_;
	int n = data->n;
	int n1 = data->n1;
	int m = data->m;
	int s = data->s;
	int S = data->S;
	simResult * results = data->SimResults;
	ExecType execType = data->execType;

	//Declare streams & vars 
	clrngMrg31k3pStream* stream_demand = NULL, *stream_order = NULL;
	clrngMrg31k3pStream *substreams_demand = NULL, *substreams_order = NULL;

	clrngStatus err;
	size_t streamBufferSize;
	size_t NbrStreams = ((execType == basic || execType == Case_a) ? 1 : n);

	//Create profit stat
	double *stat_profit = (double *)malloc(n * sizeof(double));

	//Creator used to reset the state of the base seed in case there is successive calls to the same "Option"
	clrngMrg31k3pStreamCreator* Creator = clrngMrg31k3pCopyStreamCreator(NULL, &err);
	check_error(err, "%s(): cannot create stream creator", __func__);

	

	//Create stream demand
	if (execType == basic || execType == Case_a || execType == Case_b) {		
		stream_demand = clrngMrg31k3pCreateStreams(Creator, NbrStreams, &streamBufferSize, &err);
		check_error(err, "%s(): cannot create random stream demand", __func__);
	}

	//*************************
	//Simulate on CPU
	if (execType == basic)
	{
		// in the document, this corresponds to the call to
		// inventorySimulateRunsOneStream()
		inventorySimulateRunsCPU(m, s, S, n, stream_demand, NULL, stat_profit, execType, results);

	}
	else if (execType == Case_a || execType == Case_b)
	{
		//Set the result object ExecOption
		if (results != NULL){
			if (execType == Case_a) results->ExecOption = 2;
			else results->ExecOption = 3;
		}

		stream_order = clrngMrg31k3pCreateStreams(Creator, NbrStreams, &streamBufferSize, &err);
		check_error(err, "%s(): cannot create random stream order", __func__);

		// in the document, this corresponds to the calls to
		// inventorySimulateRunsManyStreams()
		// inventorySimulateRunsSubstreams()
		inventorySimulateRunsCPU(m, s, S, n, stream_demand, stream_order, stat_profit, execType, results);
	}

	//*************************
	//Simulate on Device
	if (execType != basic){

		if (stream_demand != NULL)
		    clrngMrg31k3pRewindStreams(NbrStreams, stream_demand);
		if (stream_order != NULL)
		    clrngMrg31k3pRewindStreams(NbrStreams, stream_order);

		if (execType == Case_a)
		{
			if (results != NULL) (&results[3])->ExecOption = 2;
			printf("\n+++++++++     On Device (case a) : One policy, two streams with their substreams \n");
			substreams_demand = clrngMrg31k3pMakeSubstreams(stream_demand, n, &streamBufferSize, &err);
			check_error(err, "%s(): cannot create random substreams demand", __func__);

			substreams_order = clrngMrg31k3pMakeSubstreams(stream_order, n, &streamBufferSize, &err);
			check_error(err, "%s(): cannot create random substreams order", __func__);

			inventorySimulateRunsGPU(context, device, queue, m, &s, &S, 1, n, 0, 0, OnePolicy, "inventorySimulateGPU", streamBufferSize, substreams_demand, substreams_order, stat_profit, (results != NULL?&results[3]:NULL));
		
		}
		else if (execType == Case_b)
		{
			if (results != NULL) (&results[3])->ExecOption = 3;
			printf("\n+++++++++     On Device (case b) : One policy, two arrays of n streams each \n");
			inventorySimulateRunsGPU(context, device, queue, m, &s, &S, 1, n, 0, 0, OnePolicy, "inventorySimulateGPU", streamBufferSize, stream_demand, stream_order, stat_profit, (results != NULL ? &results[3] : NULL));
		}
		else if (execType == Case_c)
		{
			if (results != NULL) results->ExecOption = 4;
			printf("\n+++++++++     On Device (case c) : One policy, using 2*n1 streams with n2 substreams on each \n");
			stream_demand = clrngMrg31k3pCreateStreams(Creator, n1, &streamBufferSize, &err);
			check_error(err, "%s(): cannot create random stream demand", __func__);

			stream_order = clrngMrg31k3pCreateStreams(Creator, n1, &streamBufferSize, &err);
			check_error(err, "%s(): cannot create random stream order", __func__);

			inventorySimulateRunsGPU(context, device, queue, m, &s, &S, 1, n, n1, n / n1, OnePolicy, "inventorySimulSubstreamsGPU", streamBufferSize, stream_demand, stream_order, stat_profit, results);
		}
		else if (execType == Case_d)
		{
			if (results != NULL) results->ExecOption = 5;
			printf("\n+++++++++     On Device (case d) : One policy, using 2*n streams with 2*n2 streams per Work item \n");
			stream_demand = clrngMrg31k3pCreateStreams(Creator, n, &streamBufferSize, &err);
			check_error(err, "%s(): cannot create random stream demand", __func__);

			stream_order = clrngMrg31k3pCreateStreams(Creator, n, &streamBufferSize, &err);
			check_error(err, "%s(): cannot create random stream order", __func__);

			inventorySimulateRunsGPU(context, device, queue, m, &s, &S, 1, n, n1, n / n1, OnePolicy, "inventorySimul_DistinctStreams_GPU", streamBufferSize, stream_demand, stream_order, stat_profit, results);
		}
	}

	//Free resources
	free(stat_profit);
	clrngMrg31k3pDestroyStreams(stream_demand);
	clrngMrg31k3pDestroyStreams(stream_order);
	clrngMrg31k3pDestroyStreams(substreams_demand);
	clrngMrg31k3pDestroyStreams(substreams_order);
	clrngMrg31k3pDestroyStreamCreator(Creator);

	return EXIT_SUCCESS;
}

int several_Policies(cl_context context, cl_device_id device, cl_command_queue queue, void* data_)
{
	SeveralPoliciesData* data = (SeveralPoliciesData*)data_;
	int n = data->n;
	int n1 = data->n1;
	int m = data->m;
	int* s = data->s;
	int* S = data->S;
	int P = data->P;
	simResult * results = data->SimResults;
	ExecOption _optionType = data->optionType;

	//Declare streams & vars
	clrngMrg31k3pStream* streams_demand = NULL, *streams_order = NULL;
	double *stat_profit = NULL, *stat_diff = NULL;

	clrngStatus err;
	size_t streamBufferSize;

	//Allocate stat profit for  n*P runs
	stat_profit = (double *)malloc(n * P * sizeof(double));

	//Creator used to reset the state of the base seed in case there is successive calls to the same "Option"
	clrngMrg31k3pStreamCreator* Creator = clrngMrg31k3pCopyStreamCreator(NULL, &err);
	check_error(err, "%s(): cannot create stream creator", __func__);

	//Create n1 Streams for demand and order
	streams_demand = clrngMrg31k3pCreateStreams(Creator, n1, &streamBufferSize, &err);
	check_error(err, "%s(): cannot create random streams demand", __func__);

	streams_order = clrngMrg31k3pCreateStreams(Creator, n1, &streamBufferSize, &err);
	check_error(err, "%s(): cannot create random streams order", __func__);

	//printf("\n++++++++++++++++++++++++++++++     On Device   +++++++++++++++++++++++++++++++\n");
	if (_optionType == Option1)
	{
		printf("+++++++++     Simulate n2 runs on n1 work items using 2*n1 streams and n2 substreams for each, for P policies in series \n");
		printf("+++++++++     CRN simulation : \n");
		for (int k = 0; k < P; k++) {
			inventorySimulateRunsGPU(context, device, queue, m, &s[k], &S[k], P, n, n1, n/n1, Option1, "inventorySimulSubstreamsGPU", streamBufferSize, streams_demand, streams_order, &stat_profit[k*n], NULL);
			clrngMrg31k3pRewindStreams(n1, streams_demand);
			clrngMrg31k3pRewindStreams(n1, streams_order);
		}
		//Compute CI
		stat_diff = (double *)malloc(n * sizeof(double));
		for (int i = 0; i < n; i++)
			stat_diff[i] = stat_profit[n + i] - stat_profit[i];
		printf("\nDifference CRN :\n ------------\n");
		computeCI(n, stat_diff, NULL);

		printf("\n+++++++++     IRN simulation : \n");
		for (int k = 0; k < P; k++) {
			inventorySimulateRunsGPU(context, device, queue, m, &s[k], &S[k], P, n, n1, n / n1, Option1, "inventorySimulSubstreamsGPU", streamBufferSize, streams_demand, streams_order, &stat_profit[k*n], NULL);

			streams_demand = clrngMrg31k3pCreateStreams(NULL, n1, &streamBufferSize, &err);
			check_error(err, "%s(): cannot create random streams demand", __func__);

			streams_order = clrngMrg31k3pCreateStreams(NULL, n1, &streamBufferSize, &err);
			check_error(err, "%s(): cannot create random streams order", __func__);

		}
		//Compute CI
		for (int i = 0; i < n; i++)
			stat_diff[i] = stat_profit[n + i] - stat_profit[i];
		printf("\nDifference IRN:\n ------------\n");
		computeCI(n, stat_diff , NULL);
	}
	else { //Option2 
		
		if (results != NULL) results->ExecOption = 7;
		
		//printf("+++++++++     Simulate n2 runs on n1p workitmes using n1 streams and n2 substreams, all P policies in parallel\n");
		inventorySimulateRunsGPU(context, device, queue, m, s, S, P, n * P, n1 * P, n/n1, Option2, "inventorySimulPoliciesGPU",
			                streamBufferSize, streams_demand, streams_order, stat_profit, results);

		//Compute CI
		stat_diff = (double *)malloc(n * sizeof(double));
		int n2 = n / n1;
		for (int j = 0; j < n2; j++)
			for (int i = 0; i < n1; i++)
			{
				int index = i + (j*n1*P);
				stat_diff[i + j*n1] = stat_profit[n1 + index] - stat_profit[index];
			}
		//printf("\nDifference:\n ------------\n");
		computeCI(n, stat_diff, results);
	}

	//Free Resources
	
	clrngMrg31k3pDestroyStreams(streams_demand);
	clrngMrg31k3pDestroyStreams(streams_order);
	clrngMrg31k3pDestroyStreamCreator(Creator);
	free(stat_diff);
	free(stat_profit);

	return EXIT_SUCCESS;
}
