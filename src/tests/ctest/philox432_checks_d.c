
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

#define CTEST_RNG_TYPE   Philox432
#define CTEST_RNG_HEADER philox432
#include "mangle.h"

#define CTEST_SUBSTREAM_STEP_LOG2 66
#define CTEST_SUBSTREAM_STEP_LIN  0

// The following values were generated using Random123:
cl_double CTEST_MANGLE(expectedRandomU01Values)[] = {
  0.037094080704264343,
  0.693930919165723026,
  0.362091115559451282,
  0.972241201554425061,
  0.271746986662037671,
  0.140946607454679906,
  0.319445768021978438,
  0.019449422485195100,
  0.426776767824776471,
  0.604630667599849403,
  0.415107050561346114,
  0.787367770797573030
};

clrngStreamState CTEST_MANGLE(expectedSuccessiveStates)[] = {
  {{{0,0},{0,1}}, {0,0,0,0}, 0},
  {{{0,0},{0,1}}, {0,0,0,0}, 1},
  {{{0,0},{0,1}}, {0,0,0,0}, 2},
  {{{0,0},{0,1}}, {0,0,0,0}, 3},
  {{{0,0},{0,2}}, {0,0,0,0}, 0},
  {{{0,0},{0,2}}, {0,0,0,0}, 1},
  {{{0,0},{0,2}}, {0,0,0,0}, 2},
  {{{0,0},{0,2}}, {0,0,0,0}, 3},
  {{{0,0},{0,3}}, {0,0,0,0}, 0},
  {{{0,0},{0,3}}, {0,0,0,0}, 1}
};

clrngStreamState CTEST_MANGLE(expectedCreateStreamsStates)[] = {
  {{{ 0,0},{0,1}}, {0,0,0,0}, 0},
  {{{16,0},{0,1}}, {0,0,0,0}, 0},
  {{{32,0},{0,1}}, {0,0,0,0}, 0},
  {{{48,0},{0,1}}, {0,0,0,0}, 0},
  {{{64,0},{0,1}}, {0,0,0,0}, 0},
  {{{80,0},{0,1}}, {0,0,0,0}, 0}
};

clrngStreamState CTEST_MANGLE(expectedForwardToNextSubstreamsStates)[] = {
  {{{0,1},{0,1}}, {0,0,0,0}, 0},
  {{{0,2},{0,1}}, {0,0,0,0}, 0},
  {{{0,3},{0,1}}, {0,0,0,0}, 0},
  {{{0,4},{0,1}}, {0,0,0,0}, 0},
  {{{0,5},{0,1}}, {0,0,0,0}, 0}
};

fp_type CTEST_MANGLE_PREC2(expectedCombinedOperationsValue) = 38.672370329515402432;

#define CTEST_STREAMS_SPACING_MULTIPLE_OF_4

#include "checks.c.h"
#include "checks_prec.c.h"
