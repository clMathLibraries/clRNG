
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
#include <time.h>

/* uncomment to use single precision floating point numbers */
//#define CLRNG_SINGLE_PRECISION

#ifdef CLRNG_SINGLE_PRECISION
typedef cl_float fp_type;
#else
typedef cl_double fp_type;
#endif

/* Define only one of the following: */
#define USE_MRG31K3P
//#define USE_MRG32K3A
//#define USE_LFSR113

#include "../common.h"
#include "../anyrng.h"

void write_fp_array(FILE* file, size_t num_cols, const char* format, size_t array_size, const fp_type* array)
{
    size_t num_rows = (array_size + num_cols - 1) / num_cols;
    for (size_t row = 0; row < num_rows; row++) {
	for (size_t col = 0; col < num_cols; col++) {
	    size_t i = col * num_rows + row;
	    if (i < array_size)
		fprintf(file, format, array[i]);
	}
	fprintf(file, "\n");
    }
}


typedef struct TaskData_ {
    size_t number_count;
    size_t stream_count;
    int display;
} TaskData;

void multistream_fill_array(size_t stream_count, clrngStream* streams, size_t number_count, fp_type* numbers)
{
  for (size_t i = 0; i < number_count; i++)
    numbers[i] = clrngRandomU01(&streams[i % stream_count]);
}

int task(cl_context context, cl_device_id device, cl_command_queue queue, void* data_)
{
    cl_int err;
    const TaskData* data = (const TaskData*) data_;
    size_t stream_count = data->stream_count;
    size_t number_count = data->number_count;

    // prepare input and output buffers
    /*! [streams allocation] */
    size_t streams_buf_size;
    clrngStream* streams = clrngCreateStreams(NULL, stream_count, &streams_buf_size, (clrngStatus *)&err);
    /*! [streams allocation] */
    check_error(err, "cannot create random streams");

    fp_type* numbers_host = (fp_type*) malloc(number_count * sizeof(fp_type));

    // fill array on the host, using the same ordering as on the device
    clock_t host_start_time = clock();
    multistream_fill_array(stream_count, streams, number_count, numbers_host);
    clock_t host_end_time = clock();

    if (data->display) {
	// output generated values
	printf("\nArray generated on the host:\n\n");
	write_fp_array(stdout, 4, "%12.6f", number_count, numbers_host);
    }

    // output computing time
    printf("\nComputing time on the host: %1.5f s\n", (float)(host_end_time - host_start_time) / CLOCKS_PER_SEC);

    // reset streams for usage on device
    clrngRewindStreams(stream_count, streams);

    // create OpenCL buffers
    /*! [streams buffer] */
    cl_mem streams_buf = clCreateBuffer(context, CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, streams_buf_size, streams, &err);
    /*! [streams buffer] */
    check_error(err, "cannot create streams buffer");
    /*! [numbers buffer] */
    cl_mem numbers_buf = clCreateBuffer(context, CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY, number_count * sizeof(fp_type), NULL, &err);
    /*! [numbers buffer] */
    check_error(err, "cannot create numbers buffer");

    // fill array on the device
    /*! [random array call] */
    cl_event event;
    err = clrngDeviceRandomU01Array(stream_count, streams_buf, number_count, numbers_buf, 1, &queue, 0, NULL, &event);
    /*! [random array call] */
    check_error(err, NULL);
    /*! [random array wait] */
    err = clWaitForEvents(1, &event);
    /*! [random array wait] */
    check_error(err, "error waiting for events");

    // retrieve output values
    /*! [random array read] */
    fp_type* numbers = (fp_type*) malloc(number_count * sizeof(fp_type));
    err = clEnqueueReadBuffer(queue, numbers_buf, CL_TRUE, 0, number_count * sizeof(fp_type), numbers, 0, NULL, NULL);
    /*! [random array read] */
    check_error(err, "cannot read output buffer");

    if (data->display) {
	// output generated values
	printf("\nArray generated on the device:\n\n");
	write_fp_array(stdout, 4, "%12.6f", number_count, numbers);
    }

    // output computing time
    cl_ulong device_time_start, device_time_end;
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(device_time_start), &device_time_start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,   sizeof(device_time_end),   &device_time_end, NULL);
    printf("\nComputing time on the device: %1.5f s\n", (device_time_end - device_time_start) / 1.0e9);

    // compare outputs from host and device
    size_t diff_count = 0;
    fp_type diff_tot = 0.0;
    for (size_t j = 0; j < number_count; j++) {
	fp_type n1 = numbers_host[j];
	fp_type n2 = numbers[j];
	if (n1 != n2) {
	    diff_tot += n1 > n2 ? n1 - n2 : n2 - n1;
	    diff_count++;
	}
    }
    printf("\ncount of different values: %lu\n", diff_count);
    if (diff_count > 0)
	printf("average difference: %g\n", diff_tot / diff_count);

    // release resources
    free(numbers_host);
    free(numbers);
    clrngDestroyStreams(streams);
    clReleaseEvent(event);

    return EXIT_SUCCESS;
}


int main()
{
    // IMPORTANT: number_count must be a multiple of stream_count
    // Set display to a nonzero value to output the numbers
    size_t stream_count  = 1 << 10;
    size_t numbers_ratio = 1 << 10;
    TaskData data = {
	data.number_count = stream_count * numbers_ratio,
	data.stream_count = stream_count,
	data.display = 0
    };
    
    return call_with_opencl(0, CL_DEVICE_TYPE_GPU, 0, &task, &data, true);
}


