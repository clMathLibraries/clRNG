
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

/*
 * Verifies device output using multiple streams per work-item.
 */

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* uncomment to use single precision floating point numbers */
//#define CLRNG_SINGLE_PRECISION

#ifdef CLRNG_SINGLE_PRECISION
typedef cl_float fp_type;
#define DEVICE_FP_TYPE "float"
#else
typedef cl_double fp_type;
#define DEVICE_FP_TYPE "double"
#endif

/* Define only one of the following: */
#define USE_MRG31K3P
//#define USE_MRG32K3A
//#define USE_LFSR113

#include "../common.h"
#include "../anyrng.h"


#if defined(_MSC_VER) || defined(__MINGW32__)
    #define SIZE_T_FORMAT   "%Iu"
#elif defined(__GNUC__)
    #define SIZE_T_FORMAT   "%zu"
#else
    #define SIZE_T_FORMAT   "%lu"
#endif


void write_fp_array(FILE* file, size_t num_cols, const char* format, size_t array_size, const fp_type* array);
void multistream_fill_array(size_t spwi, size_t gsize, size_t quota, int substream_length, clrngStream* streams, fp_type* out);


typedef struct {
    size_t  streams_per_work_item;
    size_t  num_work_items;
    cl_uint quota_per_work_item;
    /* substream_length = 0:  do not use substreams
     * substream_length > 0:  go to next substreams after substream_length values
     * substream_length < 0:  restart substream after -substream_length values
     */
    int substream_length;
} TaskData;


int task(cl_context context, cl_device_id device, cl_command_queue queue, void* data_)
{
    cl_int err;
    const TaskData* data = (const TaskData*) data_;
    size_t total_streams = data->num_work_items * data->streams_per_work_item;
    size_t output_length = total_streams * data->quota_per_work_item;

    size_t streams_buf_size;
    clrngStream* streams = clrngCreateStreams(NULL, total_streams,
		&streams_buf_size, (clrngStatus *)&err);
    check_error(err, "cannot create random stream array");

    // generate host output
    fp_type* out1 = (fp_type*) malloc(output_length * sizeof(fp_type));
    multistream_fill_array(
            data->streams_per_work_item,
            data->num_work_items,
            data->quota_per_work_item,
            data->substream_length,
            streams,
            out1);

    // to check if the end states of streams match with those of device streams
    clrngStream* streamsCheck = clrngCopyStreams(total_streams, streams, (clrngStatus *)&err);
    check_error(err, "cannote clone streams");

    // reset streams for usage on device
    clrngRewindStreams(total_streams, streams);

    cl_mem streams_buf = clCreateBuffer(context,
            CL_MEM_COPY_HOST_PTR, 
            //CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
            streams_buf_size, streams, &err);
    check_error(err, "cannot create streams buffer");
    cl_mem out_buf = clCreateBuffer(context,
            CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY, 
            output_length * sizeof(fp_type), NULL, &err);
    check_error(err, "cannot create numbers buffer");

    char source[800];
    char buf[100] = "\000";
    if (data->substream_length > 0) {
        sprintf(buf, "        if (i > 0 && i %% %d == 0) clrng" RNG_PREFIX_S "ForwardToNextSubstreams(" SIZE_T_FORMAT ", s);\n",
                data->substream_length, data->streams_per_work_item);
    }
    else if (data->substream_length < 0) {
        sprintf(buf, "        if (i > 0 && i %% %d == 0) clrng" RNG_PREFIX_S "RewindSubstreams(" SIZE_T_FORMAT ", s);\n",
                -data->substream_length, data->streams_per_work_item);
    }
    sprintf(source,
        "%s"
#ifdef CLRNG_SINGLE_PRECISION
        "#define CLRNG_SINGLE_PRECISION\n"
#endif
        "#include " RNG_DEVICE_HEADER_S "\n"
        "#pragma OPENCL EXTENSION cl_amd_printf : enable\n"
        "#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n"
        "__kernel void example(__global clrng" RNG_PREFIX_S "HostStream* streams, uint quota, __global " DEVICE_FP_TYPE "* out) {\n"
        "    int gid = get_global_id(0);\n"
        "    int gsize = get_global_size(0);\n"
        "    clrng" RNG_PREFIX_S "Stream s[" SIZE_T_FORMAT "];\n"
        "    clrng" RNG_PREFIX_S "CopyOverStreamsFromGlobal(" SIZE_T_FORMAT ", s, &streams[" SIZE_T_FORMAT " * gid]);\n"
        "    for (uint i = 0; i < quota; i++) {\n"
        "%s"
        "        for (uint j = 0; j < " SIZE_T_FORMAT "; j++)\n"
        "           out[" SIZE_T_FORMAT " * (i * gsize + gid) + j] = clrng" RNG_PREFIX_S "RandomU01(&s[j]);\n"
        "    }\n"
        "    clrng" RNG_PREFIX_S "CopyOverStreamsToGlobal(" SIZE_T_FORMAT ", &streams[" SIZE_T_FORMAT " * gid], s);\n"
        "}\n",
        data->substream_length != 0 ? "#define CLRNG_ENABLE_SUBSTREAMS\n" : "",
        data->streams_per_work_item,
        data->streams_per_work_item,
        data->streams_per_work_item,
        buf,
        data->streams_per_work_item,
        data->streams_per_work_item,
        data->streams_per_work_item,
        data->streams_per_work_item);
    printf("source:\n\n%s\n", source);

    const char* sources[] = { source };
    cl_program program = clCreateProgramWithSource(context, 1, sources, NULL, &err);
    check_error(err, "cannot create program");

    err = clBuildProgram(program, 0, NULL, clrngGetLibraryDeviceIncludes(NULL), NULL, NULL);
    if (err < 0)
        write_build_log(stderr, program, device);
    check_error(err, "cannot build program");

    cl_kernel kernel = clCreateKernel(program, "example", &err);
    check_error(err, "cannot create kernel");

    err  = clSetKernelArg(kernel, 0, sizeof(streams_buf),                 &streams_buf);
    err |= clSetKernelArg(kernel, 1, sizeof(data->quota_per_work_item),   &data->quota_per_work_item);
    err |= clSetKernelArg(kernel, 2, sizeof(out_buf),                     &out_buf);
    check_error(err, "cannot create set kernel arguments");

    cl_event ev;
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &data->num_work_items, NULL, 0, NULL, &ev);
    check_error(err, "cannot enqueue kernel");

    err = clWaitForEvents(1, &ev);
    check_error(err, "error waiting for events");

    cl_ulong t0, t1;
    clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof(t0), &t0, NULL);
    clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END,   sizeof(t1), &t1, NULL);
    cl_ulong total_time = t1 - t0;

    // read output numbers
    fp_type* out2 = (fp_type*) malloc(output_length * sizeof(fp_type));
    err = clEnqueueReadBuffer(queue, out_buf, CL_TRUE, 0, output_length * sizeof(fp_type), out2, 0, NULL, NULL);
    check_error(err, "cannot read output buffer");

    // read streams back from device
    err = clEnqueueReadBuffer(queue, streams_buf, CL_TRUE, 0, streams_buf_size, streams, 0, NULL, NULL);
    check_error(err, "cannot read streams buffer");

    printf("\nprocessing time: %1.2f\n", total_time * 1e-9);

    clReleaseEvent(ev);
    clReleaseMemObject(streams_buf);
    clReleaseMemObject(out_buf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);

#if 0
    // display output
    printf("\nHost output:\n\n");
    write_fp_array(stdout, 4, "%12.6f", output_length, out1);
    printf("\nDevice output:\n");
    write_fp_array(stdout, 4, "%12.6f", output_length, out2);
#endif

    // compare outputs from host and device
    size_t diff_count = 0;
    fp_type diff_tot = 0.0;
    for (size_t j = 0; j < output_length; j++) {
        fp_type n1 = out1[j];
        fp_type n2 = out2[j];
        if (n1 != n2) {
            diff_tot += n1 > n2 ? n1 - n2 : n2 - n1;
            diff_count++;
        }
    }
    printf("\ncount of different values: %lu\n", diff_count);
    if (diff_count > 0)
        printf("average difference: %g\n", diff_tot / diff_count);

    // compare stream states
    diff_count = 0;
    for (size_t j = 0; j < total_streams; j++) {
        if (memcmp(&streams[j].current, &streamsCheck[j].current, sizeof(clrngStreamState)))
            diff_count++;
    }
    printf("\ncount of different stream states: %lu\n", diff_count);

    free(out1);
    free(out2);
    clrngDestroyStreams(streams);

    return EXIT_SUCCESS;
}

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


void multistream_fill_array(size_t spwi, size_t gsize, size_t quota, int substream_length, clrngStream* streams, fp_type* out_)
{
  for (size_t i = 0; i < quota; i++) {
      for (size_t gid = 0; gid < gsize; gid++) {
          clrngStream* s = &streams[spwi * gid];
          fp_type* out = &out_[spwi * (i * gsize + gid)];
          if ((i > 0) && (substream_length > 0) && (i % substream_length == 0))
              clrngForwardToNextSubstreams(spwi, s);
          else if ((i > 0) && (substream_length < 0) && (i % (-substream_length) == 0))
              clrngRewindSubstreams(spwi, s);
          for (size_t sid = 0; sid < spwi; sid++) {
              out[sid] = clrngRandomU01(&s[sid]);
          }
      }
  }
}



int main()
{
    TaskData data = {
        data.streams_per_work_item  = 2,
        data.num_work_items         = 1 << 10,
        data.quota_per_work_item    = 1 << 10,
        data.substream_length       = 0
        //.substream_length       = 0   // do not use substreams
        //.substream_length       = 8   // go to next substreams after 8 values
        //.substream_length       = -8  // restart substream after 8 values
    };
    return call_with_opencl(0, CL_DEVICE_TYPE_GPU, 0, &task, &data, true);
}
