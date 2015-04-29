
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef __APPLE__
#include <Opencl/cl.h>
#else
#include <CL/cl.h>
#endif

#include "util.h"

// defined in cltest_<rng>.c
int ctestMrg31k3p_dispatch (const DeviceSelect* dev);
int ctestMrg32k3a_dispatch (const DeviceSelect* dev);
int ctestLfsr113_dispatch  (const DeviceSelect* dev);
int ctestPhilox432_dispatch(const DeviceSelect* dev);

// defined below in this file
int usage();

// for use from within usage()
static const char* global_prog;


// main program: command line interface (CLI)
int main(int argc, char** argv)
{
  global_prog = portable_basename(*argv++); argc--;

  while (argc >= 1 && (*argv)[0] == '-') {
    if (strcmp(*argv, "-h") == 0)
      return usage();
    else if (strcmp(*argv, "-v") == 0)
      ctestVerbose++;
    else {
      fprintf(stderr, "ERROR: unknown switch `%s'\n", *argv);
      return usage();
    }
    argv++; argc--;
  }

  // parse integer arguments
  size_t nargs = 0;
  long args[10];
  while (argc > 0 && nargs < (sizeof(args)/sizeof(args[0]))) {
    const char* s = *argv++; argc--;
    if (strcmp(s, "CPU") == 0)
      args[nargs++] = CL_DEVICE_TYPE_CPU;
    else if (strcmp(s, "GPU") == 0)
      args[nargs++] = CL_DEVICE_TYPE_GPU;
    else {
      args[nargs++] = strtol(s, (char**) NULL, 10);
      if (errno) {
        fprintf(stderr, "error interpreting %s as an integer\n", s);
        return EXIT_FAILURE;
      }
    }
  }

  const DeviceSelect* dev = parse_device_select(nargs, args);
  clinfo(dev);

  // launch tests
  int ret = 0;
  ret |= ctestMrg31k3p_dispatch (dev);
  ret |= ctestMrg32k3a_dispatch (dev);
  ret |= ctestLfsr113_dispatch  (dev);
  ret |= ctestPhilox432_dispatch(dev);
  return ret;
}


// display usage directives
int usage()
{
  fprintf(stderr, "usage:\n");
  fprintf(stderr, "\n  run tests:\n");
  fprintf(stderr, "    %s [-v] [-v] [CPU|GPU] [<device>] [<platform>]\n", global_prog);
  fprintf(stderr, "\n  display help:\n");
  fprintf(stderr, "    %s -h\n", global_prog);
  fprintf(stderr, "\nwhere:\n");
  fprintf(stderr, "             <device>: device index (starting from 0)\n");
  fprintf(stderr, "           <platvorm>: device index (starting from 0)\n");
  fprintf(stderr, "      the `-v' switch: enables verbose output (can be used twice)\n");
  return EXIT_FAILURE;
}
