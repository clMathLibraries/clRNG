
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

/*! @file checks_prec.c.h
 *  @brief Tests that depend on the floating-point precision.
 *
 *  These tests must be compiled twice for every generator: with and
 *  without CLRNG_SINGLE_PRECISION.
 */

#include "mangle.h"
#include "util.h"
#include "../client/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define SUCCESS_STR(ret) ((ret) == EXIT_SUCCESS ? "  SUCCESS" : "* FAILURE")
#ifdef CLRNG_SINGLE_PRECISION
#define PREC_STR "float"
#else
#define PREC_STR "double"
#endif


// defined in *_common.c
cl_long CTEST_MANGLE(compareState)(const clrngStreamState* state1, const clrngStreamState* state2);
void CTEST_MANGLE(writeState)(FILE* file, const clrngStreamState* state);
// for MSVC
cl_long ctestMrg31k3p_compareState(const clrngStreamState* state1, const clrngStreamState* state2);
void    ctestMrg31k3p_writeState  (FILE* file, const clrngStreamState* state);
cl_long ctestMrg32k3a_compareState(const clrngStreamState* state1, const clrngStreamState* state2);
void    ctestMrg32k3a_writeState  (FILE* file, const clrngStreamState* state);
cl_long ctestLfsr113_compareState(const clrngStreamState* state1, const clrngStreamState* state2);
void    ctestLfsr113_writeState  (FILE* file, const clrngStreamState* state);
cl_long ctestPhilox432_compareState(const clrngStreamState* state1, const clrngStreamState* state2);
void    ctestPhilox432_writeState  (FILE* file, const clrngStreamState* state);


#ifndef CLRNG_SINGLE_PRECISION
/*! @brief Compares U01 values with expected values.
 *
 *  With verbose >= 2: Output successive values.
 */
int CTEST_MANGLE_PREC2(checkRandomU01)()
{
  int ret = EXIT_SUCCESS;
  size_t num_states = CTEST_ARRAY_SIZE(CTEST_MANGLE(expectedRandomU01Values));
  clrngStatus err;

  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);

  clrngStream* stream = clrngCreateStreams(creator, 1, NULL, &err);
  check_error(err, NULL);

  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);

  for (size_t i = 0; i < num_states && ret == EXIT_SUCCESS; i++) {
    fp_type u = clrngRandomU01(stream);
    fp_type u2 = (fp_type) CTEST_MANGLE(expectedRandomU01Values)[i];
    if (ctestVerbose >= 2)
        printf("%8s%.18f\n", "", u);
    if (u != u2) {
      ret = EXIT_FAILURE;
      if (ctestVerbose) {
        printf("\n%4sValues do not match at iteration %" SIZE_T_FORMAT ".\n", "", i);
        printf("%4sWith clrngRandomU01():\n", "");
        printf("%8s%.18f\n", "", u);
        printf("%4sExpected value:\n", "");
        printf("%8s%.18f\n", "", u2);
        printf("\n");
      }
    }
  }

  err = clrngDestroyStreams(stream);
  check_error(err, NULL);

  printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s clrngRandomU01() [%s]\n",
      SUCCESS_STR(ret), num_states, RNG_TYPE_S, PREC_STR);

  return ret;
}


/*! @brief Compares integer values with expected values.
 *
 *  With verbose >= 2: Output successive values.
 */
int CTEST_MANGLE_PREC2(checkRandomInteger)()
{
  int ret = EXIT_SUCCESS;

  struct bounds_type {
    cl_int low;
    cl_int high;
  };
  struct bounds_type bounds[] = { {0,100}, {12345,23456}, {0,1<<30} };

  size_t num_values = CTEST_ARRAY_SIZE(CTEST_MANGLE(expectedRandomU01Values));
  size_t num_bounds = CTEST_ARRAY_SIZE(bounds);

  for (size_t i = 0; i < num_bounds; i++) {

    cl_int low = bounds[i].low;
    cl_int high = bounds[i].high;

    clrngStatus err;
    clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
    check_error(err, NULL);

    clrngStream* stream = clrngCreateStreams(creator, 1, NULL, &err);
    check_error(err, NULL);

    err = clrngDestroyStreamCreator(creator);
    check_error(err, NULL);

    for (size_t j = 0; j < num_values; j++) {

      cl_int expectedValue = low + (cl_int) (((fp_type) CTEST_MANGLE(expectedRandomU01Values)[j]) * (high - low + 1));
      cl_int value = clrngRandomInteger(stream, low, high);

      if (ctestVerbose >= 2)
        printf("%8s%20d", "", value);

      if (value != expectedValue) {
        ret = EXIT_FAILURE;
        if (ctestVerbose) {
          printf("\n%4sStates do not match at iteration %" SIZE_T_FORMAT ".\n", "", j);
          printf("%4sWith clrngRandomInteger():\n", "");
          printf("%8s%20d\n", "", value);
          printf("%4sExpected value:\n", "");
          printf("%8s%20d\n", "", expectedValue);
          printf("\n");
        }
        break;
      }
    }

    err = clrngDestroyStreams(stream);
    check_error(err, NULL);

    printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s clrngRandomInteger(%d,%d) [%s]\n",
        SUCCESS_STR(ret), (num_values * num_bounds), RNG_TYPE_S, low, high, PREC_STR);
  }

  return ret;
}
#endif


/*! @brief Helper function for checkDeviceRandomArray()
 *
 *  Fill the array `output` with `output_ratio` numbers from each of the
 *  `stream_count` streams from the array `streams`.
 */
static int hostRandomArray(size_t stream_count, clrngStream* streams, size_t output_ratio, fp_type* output)
{
  size_t output_count = stream_count * output_ratio;
  for (size_t i = 0; i < output_count; i++)
    output[i] = clrngRandomU01(&streams[i % stream_count]);
  return EXIT_SUCCESS;
}

/*! @brief Structure for use with deviceRandomArray()
 */
typedef struct DeviceRandomArrayParams_ {
  size_t        stream_count;
  clrngStream*  streams;
  size_t        output_ratio;
  fp_type*      output;
} DeviceRandomArrayParams;

/*! @brief Helper function for checkDeviceRandomArray()
 */
static int deviceRandomArray(cl_context context, cl_device_id device, cl_command_queue queue, void* data_)
{
  const DeviceRandomArrayParams* data = (const DeviceRandomArrayParams*) data_;
  size_t output_count = data->stream_count * data->output_ratio;

  cl_int err;
  cl_mem streams_buf = clCreateBuffer(context,
      CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      data->stream_count * sizeof(clrngStream), data->streams, &err);
  check_error(err, "cannot create streams buffer");
  cl_mem numbers_buf = clCreateBuffer(context,
      CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY,
      output_count * sizeof(fp_type), NULL, &err);
  check_error(err, "cannot create numbers buffer");

  // fill array on the device
  cl_event event;
  err = clrngDeviceRandomU01Array(data->stream_count, streams_buf, output_count, numbers_buf, 1, &queue, 0, NULL, &event);
  check_error(err, NULL);
  err = clWaitForEvents(1, &event);
  check_error(err, "error waiting for events");

  // retrieve output values
  err = clEnqueueReadBuffer(queue, numbers_buf, CL_TRUE, 0, output_count * sizeof(fp_type), data->output, 0, NULL, NULL);
  check_error(err, "cannot read output buffer");

  return EXIT_SUCCESS;
}

/*! @brief Fill and compare arrays of random numbers generated on the host and device.
 *
 *  Fill two arrays of real numbers with `output_ratio` numbers from each of
 *  the `stream_count` streams stored in the array `streams`.
 *  On array is filled on the host; the other is filled on the device.
 *  The arrays are compared.
 *
 *  With verbose `>= 2`: Display the array after filling.
 */
int CTEST_MANGLE_PREC2(checkDeviceRandomArrayHelper)(
    size_t              stream_count,
    clrngStream*        streams,
    size_t              output_ratio,
    const DeviceSelect* dev)
{
  size_t output_count = stream_count * output_ratio;
  fp_type* device_output = (fp_type*) malloc(output_count * sizeof(fp_type));
  fp_type* host_output   = (fp_type*) malloc(output_count * sizeof(fp_type));

  // NOTE: if device stream write-back is used in the future, we may have to
  // create a copy of the streams to pass the same streams to the host
  // function.

  DeviceRandomArrayParams params = { stream_count, streams, output_ratio, device_output };
  call_with_opencl(dev->platform_index, dev->device_type, dev->device_index, &deviceRandomArray, &params, false);
  hostRandomArray(stream_count, streams, output_ratio, host_output);

  int ret = EXIT_SUCCESS;
  for (size_t i = 0; i < output_count && ret == EXIT_SUCCESS; i++) {
    if (device_output[i] != host_output[i]) {
      if (ctestVerbose) {
        printf("\n%4sValues do not match at output index %" SIZE_T_FORMAT ".\n", "", i);
        printf("%4sFrom device:\n", "");
        printf("%8s%16.12f\n", "", device_output[i]);
        printf("%4sFrom host:\n", "");
        printf("%8s%16.12f\n", "", host_output[i]);
        printf("\n");
      }
      ret = EXIT_FAILURE;
    }
  }

  if (ctestVerbose >= 2)
    write_array(stdout, 4, "%16.12f", output_count, device_output);

  free(device_output);
  free(host_output);

  printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s clrngDeviceRandomU01Array() [%s]\n",
      SUCCESS_STR(ret), output_count, RNG_TYPE_S, PREC_STR);

  return ret;
}

int CTEST_MANGLE_PREC2(checkDeviceRandomArray)(const DeviceSelect* dev)
{
  size_t stream_count = 1 << 8;
  size_t output_ratio = 1 << 8;

  clrngStatus err;
  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);

  clrngStream* streams = clrngCreateStreams(creator, stream_count, NULL, &err);
  check_error(err, NULL);

  int ret = CTEST_MANGLE_PREC2(checkDeviceRandomArrayHelper)(stream_count, streams, output_ratio, dev);

  err = clrngDestroyStreams(streams);
  check_error(err, NULL);

  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);

  return ret;
}



/*! @brief Helper function for checkDeviceOperations()
 */
static int hostOperations(size_t spwi, size_t gsize, size_t quota, int substream_length, clrngStream* streams, fp_type* out_)
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
  return EXIT_SUCCESS;
}

/*! @brief Structure for use with deviceOperations()
 */
typedef struct DeviceOperationsParams_ {
  size_t        streams_per_work_item;
  size_t        num_work_items;
  cl_uint       quota_per_work_item;
  cl_int        substream_length;
  size_t        streams_size;
  clrngStream*  streams;
  fp_type*      output;
} DeviceOperationsParams;

/*! @brief Helper function for checkDeviceOperations()
*/
static int deviceOperations(cl_context context, cl_device_id device, cl_command_queue queue, void* data_)
{
  cl_int err;
  const DeviceOperationsParams* data = (const DeviceOperationsParams*) data_;
  size_t total_streams = data->num_work_items * data->streams_per_work_item;
  size_t output_count = total_streams * data->quota_per_work_item;
  fp_type* output = data->output;

  cl_mem streams_buf = clCreateBuffer(context,
      CL_MEM_COPY_HOST_PTR, 
      //CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
      data->streams_size, data->streams, &err);
  check_error(err, "cannot create streams buffer");
  cl_mem out_buf = clCreateBuffer(context,
      CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY, 
      output_count * sizeof(fp_type), NULL, &err);
  check_error(err, "cannot create numbers buffer");

  char source[800];
  char buf[100] = "\000";
  if (data->substream_length > 0) {
    sprintf(buf, "        if (i > 0 && i %% %d == 0) clrng" RNG_TYPE_S "ForwardToNextSubstreams(%" SIZE_T_FORMAT ", s);\n",
        data->substream_length, data->streams_per_work_item);
  }
  else if (data->substream_length < 0) {
    sprintf(buf, "        if (i > 0 && i %% %d == 0) clrng" RNG_TYPE_S "RewindSubstreams(%" SIZE_T_FORMAT ", s);\n",
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
      "__kernel void example(__global clrng" RNG_TYPE_S "HostStream* streams, uint quota, __global " DEVICE_FP_TYPE "* out) {\n"
      "    int gid = get_global_id(0);\n"
      "    int gsize = get_global_size(0);\n"
      "    clrng" RNG_TYPE_S "Stream s[%" SIZE_T_FORMAT "];\n"
      "    clrng" RNG_TYPE_S "CopyOverStreamsFromGlobal(%" SIZE_T_FORMAT ", s, &streams[%" SIZE_T_FORMAT " * gid]);\n"
      "    for (uint i = 0; i < quota; i++) {\n"
      "%s"
      "        for (uint j = 0; j < %" SIZE_T_FORMAT "; j++)\n"
      "           out[%" SIZE_T_FORMAT " * (i * gsize + gid) + j] = clrng" RNG_TYPE_S "RandomU01(&s[j]);\n"
      "    }\n"
      "    clrng" RNG_TYPE_S "CopyOverStreamsToGlobal(%" SIZE_T_FORMAT ", &streams[%" SIZE_T_FORMAT " * gid], s);\n"
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

  if (ctestVerbose >= 2)
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

  // read output numbers
  err = clEnqueueReadBuffer(queue, out_buf, CL_TRUE, 0, output_count * sizeof(fp_type), output, 0, NULL, NULL);
  check_error(err, "cannot read output buffer");

  // read streams back from device
  err = clEnqueueReadBuffer(queue, streams_buf, CL_TRUE, 0, data->streams_size, data->streams, 0, NULL, NULL);
  check_error(err, "cannot read streams buffer");

  clReleaseEvent(ev);
  clReleaseMemObject(streams_buf);
  clReleaseMemObject(out_buf);
  clReleaseKernel(kernel);
  clReleaseProgram(program);

  return EXIT_SUCCESS;
}

/*! @brief Fill and compare arrays of random numbers generated on the host and
 *  device using multiple streams and optionally substreams.
 *
 *  Fill two arrays of real numbers with `quota_per_work_item` numbers from each of
 *  the `streams_per_work_item` streams, stored in the array `streams`, for
 *  each of `num_work_items` work items.
 *  One array is filled on the host; the other is filled on the device.
 *  Then, the arrays are compared.
 *
 *  - If `substream_length = 0`, substreams are not used.
 *  - If `substream_length > 0`, streams are advanced to the next substream
 *    after `substream_length` values are generated.
 *  - If `substream_length < 0`, streams are rewound `-substream_length` values
 *    are generated.
 *
 *  With verbose `>= 2`: Display the array after filling.
 */
int CTEST_MANGLE_PREC2(checkDeviceOperationsHelper)(
  size_t  streams_per_work_item,
  size_t  num_work_items,
  cl_uint quota_per_work_item,
  cl_int  substream_length,
  const DeviceSelect* dev)
{
  size_t total_streams = num_work_items * streams_per_work_item;
  size_t output_count = total_streams * quota_per_work_item;

  fp_type* device_output = (fp_type*) malloc(output_count * sizeof(fp_type));
  fp_type* host_output   = (fp_type*) malloc(output_count * sizeof(fp_type));

  clrngStatus err;
  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);

  size_t streams_size;
  clrngStream* device_streams = clrngCreateStreams(NULL, total_streams, &streams_size, (clrngStatus *)&err);
  check_error(err, NULL);
  clrngStream* host_streams = clrngCopyStreams(total_streams, device_streams, (clrngStatus *)&err);
  check_error(err, NULL);

  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);

  DeviceOperationsParams params = {
    streams_per_work_item,
    num_work_items,
    quota_per_work_item,
    substream_length,
    streams_size,
    device_streams,
    device_output
  };
  call_with_opencl(dev->platform_index, dev->device_type, dev->device_index, &deviceOperations, &params, false);

  hostOperations(
      streams_per_work_item,
      num_work_items,
      quota_per_work_item,
      substream_length,
      host_streams,
      host_output);

  int ret = EXIT_SUCCESS;

  // compare output values
  for (size_t i = 0; i < output_count && ret == EXIT_SUCCESS; i++) {
    if (device_output[i] != host_output[i]) {
      if (ctestVerbose) {
        printf("\n%4sValues do not match at output index %" SIZE_T_FORMAT ".\n", "", i);
        printf("%4sFrom device:\n", "");
        printf("%8s%16.12f\n", "", device_output[i]);
        printf("%4sFrom host:\n", "");
        printf("%8s%16.12f\n", "", host_output[i]);
        printf("\n");
      }
      ret = EXIT_FAILURE;
      break;
    }
  }

  // compare stream states
  for (size_t i = 0; i < total_streams; i++) {
    if (CTEST_MANGLE(compareState)(&device_streams[i].current, &host_streams[i].current)) {
      if (ctestVerbose) {
        printf("\n%4sStates do not match at stream %" SIZE_T_FORMAT ".\n", "", i);
        printf("%4sFrom device:\n", "");
        printf("%8s", "");
        CTEST_MANGLE(writeState)(stdout, &device_streams[i].current);
        printf("%4sFrom host:\n", "");
        printf("%8s", "");
        CTEST_MANGLE(writeState)(stdout, &host_streams[i].current);
        printf("\n");
      }
      ret = EXIT_FAILURE;
      break;
    }
  }

  if (ctestVerbose >= 2)
    write_array(stdout, 4, "%16.12f", output_count, device_output);

  free(device_output);
  free(host_output);
  err = clrngDestroyStreams(device_streams);
  check_error(err, NULL);
  err = clrngDestroyStreams(host_streams);
  check_error(err, NULL);

  printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s device operations (%" SIZE_T_FORMAT ",%" SIZE_T_FORMAT ",%u,%d) [%s]\n",
      SUCCESS_STR(ret), output_count + total_streams, RNG_TYPE_S,
      streams_per_work_item, num_work_items,
      quota_per_work_item, substream_length, PREC_STR);

  return ret;
}

int CTEST_MANGLE_PREC2(checkDeviceOperations)(const DeviceSelect* dev)
{
  size_t  streams_per_work_item = 3;
  size_t  num_work_items        = 1 << 10;
  cl_uint quota_per_work_item   = 32;
  cl_int  substream_length[]    = { 0, 8, -8 };

  int ret = EXIT_SUCCESS;

  for (size_t i = 0; i < CTEST_ARRAY_SIZE(substream_length); i++) {
    ret |= CTEST_MANGLE_PREC2(checkDeviceOperationsHelper)(
        streams_per_work_item,
        num_work_items,
        quota_per_work_item,
        substream_length[i],
        dev);
  }

  return ret;
}


#ifdef CLRNG_SINGLE_PRECISION
#define fabs_fp_type        fabsf
#define FP_TYPE_EPSILON     FLT_EPSILON
#else
#define fabs_fp_type        fabs
#define FP_TYPE_EPSILON     DBL_EPSILON
#endif

/*! @brief Carry multiple operations on a stream, combine multiple generated values and check the result.
 */
int CTEST_MANGLE_PREC2(checkCombinedOperations)()
{
  fp_type sum;
  fp_type sum3;
  size_t sumi;

  clrngStatus err;
  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);

  clrngStream* stream1 = clrngCreateStreams(creator, 1, NULL, &err);
  check_error(err, NULL);
  clrngStream* stream2 = clrngCreateStreams(creator, 1, NULL, &err);
  check_error(err, NULL);
  clrngStream* stream3 = clrngCreateStreams(creator, 1, NULL, &err);
  check_error(err, NULL);

  sum = clrngRandomU01(stream2) + clrngRandomU01(stream3);

  clrngAdvanceStreams(1, stream1, 5, 3);   
  sum += clrngRandomU01(stream1);

  clrngRewindStreams(1, stream1);
  for (size_t i = 0;  i < 35; i++)
    clrngAdvanceStreams(1, stream1, 0, 1);
  sum += clrngRandomU01(stream1);

  clrngRewindStreams(1, stream1);
  sumi = 0;
  for (size_t i = 0;  i < 35; i++)
    sumi += clrngRandomInteger(stream1, 1, 10);
  sum += sumi / 100.0;

  sum3 = 0.0;
  for (size_t i = 0;  i < 100;  i++) {
    sum3 += clrngRandomU01(stream3);
  }
  sum += sum3 / 10.0;

  clrngRewindStreams(1, stream3);
  for (size_t i = 1; i <= 5; i++)
    sum += clrngRandomU01(stream3);

  for (size_t i = 0; i < 4; i++)
    clrngForwardToNextSubstreams(1, stream3);
  for (size_t i = 0; i < 5; i++)
    sum += clrngRandomU01(stream3);

  clrngRewindSubstreams(1, stream3);
  for (size_t i = 0; i < 5; i++)
    sum += clrngRandomU01(stream3);

  clrngForwardToNextSubstreams(1, stream2);
  sum3 = 0.0;
  for (size_t i = 1; i <= 100000; i++)
    sum3 += clrngRandomU01(stream2);
  sum += sum3 / 10000.0;

  sum3 = 0.0;
  for (size_t i = 1; i <= 100000; i++)
    sum3 += clrngRandomU01(stream3);
  sum += sum3 / 10000.0;

  err = clrngDestroyStreams(stream1);
  check_error(err, NULL);
  err = clrngDestroyStreams(stream2);
  check_error(err, NULL);
  err = clrngDestroyStreams(stream3);
  check_error(err, NULL);

  // arrays

  clrngStream* streams = clrngCreateStreams(creator, 4, NULL, &err);
  check_error(err, NULL);

  for (size_t i = 0; i < 4; i++)
    sum += clrngRandomU01(&streams[i]);

  clrngAdvanceStreams(1, &streams[2], -127, 0);
  sum += clrngRandomU01(&streams[2]);

  clrngForwardToNextSubstreams(1, &streams[2]);
  sum3 = 0.0;
  for (size_t i = 0; i < 100000; i++)
    sum3 += clrngRandomU01(&streams[2]);
  sum += sum3 / 10000.0;

  sum3 = 0.0;
  for (size_t i = 0; i < 100000; i++)
    sum3 += clrngRandomU01(&streams[2]);
  sum += sum3 / 10000.0;

  for (size_t i = 0; i < 4; i++)
    sum += clrngRandomU01(&streams[i]);

  err = clrngDestroyStreams(streams);
  check_error(err, NULL);

  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);

  int ret = EXIT_SUCCESS;
  fp_type rel_error = fabs_fp_type(sum - CTEST_MANGLE_PREC2(expectedCombinedOperationsValue))
    / CTEST_MANGLE_PREC2(expectedCombinedOperationsValue);
  if (rel_error > FP_TYPE_EPSILON) {
    if (ctestVerbose) {
      printf("\n%4sValues do not match.\n", "");
      printf("%4sComputed value:\n", "");
      printf("%8s%16.18f\n", "", sum);
      printf("%4sExpected value:\n", "");
      printf("%8s%16.18f\n", "", CTEST_MANGLE_PREC2(expectedCombinedOperationsValue));
      printf("%4sRelative difference > float-point precision:\n", "");
      printf("%8s%16.12f > %g\n", "", rel_error, FP_TYPE_EPSILON);
      printf("\n");
    }
    ret = EXIT_FAILURE;
  }

  printf("%s  %8d test   -  %10s combined operations [%s]\n",
      SUCCESS_STR(ret), 1, RNG_TYPE_S, PREC_STR);

  return ret;
}
