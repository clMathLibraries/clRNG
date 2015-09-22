
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

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "../common.h"


/*! [clRNG header] */
#include <clRNG/mrg31k3p.h>
/*! [clRNG header] */

int task(cl_context context, cl_device_id device, cl_command_queue queue, void* data)
{
    cl_int err;
    size_t numWorkItems = *(size_t*)data;

    /*! [create streams] */
    size_t streamBufferSize;
    clrngMrg31k3pStream* streams = clrngMrg31k3pCreateStreams(NULL, numWorkItems,
                               &streamBufferSize, (clrngStatus *)&err);
    check_error(err, "cannot create random stream array");
    /*! [create streams] */

    cl_program program = build_program_from_file(context, device,
	    "client/WorkItem/workitem_kernel.cl", PATH_RELATIVE_TO_LIB, NULL);
    cl_kernel kernel = clCreateKernel(program, "example", &err);
    check_error(err, "cannot create kernel");


    /*! [create streams buffer] */
    // Create buffer to transfer streams to the device.
    cl_mem buf_in = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                    streamBufferSize, streams, &err);
    /*! [create streams buffer] */
    check_error(err, "cannot create input buffer");
    /*! [create output buffer] */
    // Create buffer to transfer output back from the device.
    cl_mem buf_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, 
                                     numWorkItems * sizeof(cl_float), NULL, &err);
    /*! [create output buffer] */
    check_error(err, "cannot create output buffer");

    // The kernel takes two arguments; set them to buf_in, buf_out.
    err  = clSetKernelArg(kernel, 0, sizeof(buf_in),  &buf_in);
    err |= clSetKernelArg(kernel, 1, sizeof(buf_out), &buf_out);
    check_error(err, "cannot create set kernel arguments");

    // Enqueue the kernel on device.
    cl_event ev;
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &numWorkItems, NULL, 0, NULL, &ev);
    check_error(err, "cannot enqueue kernel");

    // Wait for all work items to finish.
    err = clWaitForEvents(1, &ev);
    check_error(err, "error waiting for events");

    cl_ulong t0, t1;
    clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof(t0), &t0, NULL);
    clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END,   sizeof(t1), &t1, NULL);
    cl_ulong total_time = t1 - t0;

    // Retrieve the contents of the output buffer from the device.
    cl_float* out = (cl_float*) malloc(numWorkItems * sizeof(cl_float));
    err = clEnqueueReadBuffer(queue, buf_out, CL_TRUE, 0,
	    numWorkItems * sizeof(out[0]), out, 0, NULL, NULL);
    check_error(err, "cannot read output buffer");

    printf("output buffer:\n");
    for (int j = 0; j < numWorkItems; j++)
	printf("    %f\n", out[j]);

    printf("\nprocessing time: %1.2f\n", total_time * 1e-9);

    clrngMrg31k3pDestroyStreams(streams);
    free(out);
    clReleaseEvent(ev);
    clReleaseMemObject(buf_in);
    clReleaseMemObject(buf_out);
    clReleaseKernel(kernel);
    clReleaseProgram(program);

    return EXIT_SUCCESS;
}


int main()
{
    size_t numWorkItems = 8;
    return call_with_opencl(0, CL_DEVICE_TYPE_GPU, 0, &task, &numWorkItems, true);
}

