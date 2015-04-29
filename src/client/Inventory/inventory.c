
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
#include <time.h>
#include "../common.h"
#include "Types.h"
#include "Policies.h"


int main()
{
	printf("Please type a number followed by ENTER: \n\n");
	
	printf(" One Policy : \n");
	printf(" (1) - n runs on CPU, using one stream                                     \n");
	printf(" (2) - n runs with n work items, using two streams and their substreams    \n"); 
	printf(" (3) - n runs with n work items, using two arrays of n stream each         \n");               
	printf(" (4) - n2 runs with n1 work itmes, using 2*n1 streams their substreams     \n");
	printf(" (5) - n2 runs with n1 work items, using 2*n2 distinct streams.            \n");
	
	printf("\n Several policies  : \n");
	printf(" (6) - p policies in series, with n1 work items and n2 runs per work item\n"); /*option1*/
	printf(" (7) - p policies in parallel, with n1p work items and n2 runs per work item \n"); /*option2*/

	int m = 10;        // number of days
	int n = 1 << 20;    // number of runs
	int n1 = 1 << 10;   // number of workitems that will simulate n2 runs 

	
	//Policies values
	int s[] = { 80,  80 };
	int S[] = { 200, 198};
	int P = 2;  // number of policies

	cl_device_type device_type = CL_DEVICE_TYPE_GPU;
	int platform_index = 0;
	int device_index = 0;
	int ret = 0;
	
	int funcNbr;
	printf("\n Choice (0 to quit) : ");
	scanf(" %d", &funcNbr); getchar();
	
	//Simulate different options :
	while (funcNbr != 0)
	{
		switch (funcNbr)
		{
		case 1: {
			printf("\n===========================================================================\n ONE POLICY:\n=============\n");
			printf("+++++++++     On CPU (basic case): One policy, one stream \n");
			OnePolicyData data = { n, 0, m, s[0], S[0], basic };
			ret = call_with_opencl(platform_index, device_type, device_index, &one_Policy, &data, true);

			break;
		}
		case 2: {
			printf("\n===========================================================================\n ONE POLICY:\n=============\n");
			printf("+++++++++     On CPU (case a) : One policy, two streams with their substreams  \n");
			OnePolicyData data = { n, 0, m, s[0], S[0], Case_a };
			ret = call_with_opencl(platform_index, device_type, device_index, &one_Policy, &data, true);
			break;
		}
		case 3: {
			printf("\n===========================================================================\n ONE POLICY:\n=============\n");
			printf("+++++++++     On CPU (case b): One policy, two arrays of n streams each  \n");
			OnePolicyData data = { n, 0, m, s[0], S[0], Case_b };
			ret = call_with_opencl(platform_index, device_type, device_index, &one_Policy, &data, true);
			break;
		}
		case 4: {
			printf("\n===========================================================================\n ONE POLICY:\n=============\n");
			//no CPU implementation : (case c) 
			OnePolicyData data = { n, n1, m, s[0], S[0], Case_c };
			ret = call_with_opencl(platform_index, device_type, device_index, &one_Policy, &data, true);
			break;
		}
		case 5: {
			printf("\n===========================================================================\n ONE POLICY:\n=============\n");
			//no CPU implementation : (case d)
			OnePolicyData data = { n, n1, m, s[0], S[0], Case_d };
			ret = call_with_opencl(platform_index, device_type, device_index, &one_Policy, &data, true);
			break;
		}
		case 6: {
			printf("\n===========================================================================\n Several policies Option1:\n=========\n");
			//no CPU implementation : Several policies (option 1)
			SeveralPoliciesData data = { n, n1, m, s, S, P, Option1 };
			ret = call_with_opencl(platform_index, device_type, device_index, &several_Policies, &data, true);
			break;
		}
		case 7: {
			printf("\n===========================================================================\n Several policies Option2:\n=========\n");
			//no CPU implementation : Several policies (option 2)
			SeveralPoliciesData data = { n, n1, m, s, S, P, Option2 };
			ret = call_with_opencl(platform_index, device_type, device_index, &several_Policies, &data, false);
			break;
		}

		default:
			break;
		}

		printf("\n Choice (0 to quit) : ");
		scanf(" %d", &funcNbr); 
	}

	return ret;
}
