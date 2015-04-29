
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

#define CTEST_RNG_TYPE   Mrg31k3p
#define CTEST_RNG_HEADER mrg31k3p
#include "mangle.h"

void CTEST_MANGLE(writeState)(FILE* file, const clrngStreamState* state)
{
  fprintf(file, "[ ");
  for (size_t i = 0; i < 3; i++) {
    if (i > 0) fprintf(file, ", ");
    fprintf(file, "%12d", state->g1[i]);
  }
  for (size_t i = 0; i < 3; i++) {
    fprintf(file, ", ");
    fprintf(file, "%12d", state->g2[i]);
  }
  fprintf(file, " ]\n");
}

cl_long CTEST_MANGLE(compareState)(const clrngStreamState* state1, const clrngStreamState* state2)
{
  for (size_t i = 0; i < 3; i++) {
    cl_int diff = state1->g1[i] - state2->g1[i];
    if (diff)
      return diff;
  }
  for (size_t i = 0; i < 3; i++) {
    cl_int diff = state1->g2[i] - state2->g2[i];
    if (diff)
      return diff;
  }
  return 0;
}
