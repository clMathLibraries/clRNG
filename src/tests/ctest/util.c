
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

#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include <Opencl/cl.h>
#else
#include <CL/cl.h>
#endif

#include "../client/common.h"


// globals
int ctestVerbose = 0;

// device select
static DeviceSelect device_select;
const DeviceSelect* parse_device_select(size_t nargs, const long* args)
{
  device_select.platform_index = 0;
  device_select.device_type    = CL_DEVICE_TYPE_CPU;
  device_select.device_index   = 0;
  if (nargs > 0) { device_select.device_type    = (cl_device_type) *args++; nargs--; }
  if (nargs > 0) { device_select.device_index   = (size_t)         *args++; nargs--; }
  if (nargs > 0) { device_select.platform_index = (size_t)         *args++; nargs--; }
  return &device_select;
}


// write_array

#define IMPLEMENT_WRITE_ARRAY(type) \
void write_array_##type(FILE* file, size_t num_cols, const char* format, size_t array_size, const type* array) \
{ \
  size_t num_rows = (array_size + num_cols - 1) / num_cols; \
  for (size_t row = 0; row < num_rows; row++) { \
    for (size_t col = 0; col < num_cols; col++) { \
      size_t i = col * num_rows + row; \
      if (i < array_size) \
        fprintf(file, format, array[i]); \
    } \
    fprintf(file, "\n"); \
  } \
}

IMPLEMENT_WRITE_ARRAY(cl_float)
IMPLEMENT_WRITE_ARRAY(cl_double)


char *portable_basename(char *path)
{
#ifdef _MSC_VER
  char* p = strrchr(path, '\\');
#else
  char* p = strrchr(path, '/');
#endif
  return p ? p + 1 : path;
}


static int clinfo_helper(cl_context context, cl_device_id device, cl_command_queue queue, void* data_)
{
  // Nothing to do; the call_with_opencl() wrapper will output platform and device
  // information.
  return EXIT_SUCCESS;
}

int clinfo(const DeviceSelect* dev)
{
  return call_with_opencl(dev->platform_index, dev->device_type, dev->device_index, &clinfo_helper, NULL, true);
}
