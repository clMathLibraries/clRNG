
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

#define CTEST_RNG_TYPE   Lfsr113
#define CTEST_RNG_HEADER lfsr113
#include "mangle.h"

#define CTEST_SUBSTREAM_STEP_LOG2 55
#define CTEST_SUBSTREAM_STEP_LIN  0

// The following values were generated using SSJ:
cl_double CTEST_MANGLE(expectedRandomU01Values)[] = {
  0.9202779277879097,
  0.2777645708346356,
  0.564335069953386,
  0.28643811650424306,
  0.18350138441112326,
  0.13978629323193192,
  0.6811267867029815,
  0.41023648543976327,
  0.34996541045839774,
  0.34781615870357113
};

// The following states were generated using SSJ:
clrngStreamState CTEST_MANGLE(expectedSuccessiveStates)[] = {
  {{ 987654321U,  987654321U,  987654321U,  987654321U}},
  {{2730781218U, 3950617306U, 1865703358U, 3441345234U}},
  {{2827525879U, 2917567336U, 2586828582U, 3629801101U}},
  {{2078037369U, 3080334755U,  401576718U, 1272031370U}},
  {{1441031752U, 3731404429U, 4157179692U,  890328526U}},
  {{2569103343U, 2040715828U, 3838023225U,  716822975U}}
};

// The following states were generated using SSJ:
clrngStreamState CTEST_MANGLE(expectedCreateStreamsStates)[] = {
  {{ 987654321U,  987654321U,  987654321U,  987654321U}},
  {{1238817258U, 1756794174U, 3139831156U, 3929772541U}},
  {{3976037797U, 2369672037U, 2206891917U, 1144859279U}},
  {{2208361474U, 1399180856U,  300913405U, 1189142830U}},
  {{3167838773U, 3459839895U,  905364637U, 3159382846U}}
};

// The following states were generated using SSJ:
clrngStreamState CTEST_MANGLE(expectedForwardToNextSubstreamsStates)[] = {
  {{3989748853U,  374484650U, 4095620850U, 2108357207U}},
  {{1305638194U,  331955594U, 1865703358U, 3764226144U}},
  {{2616441350U, 4073515993U,  253458839U, 1836613789U}},
  {{2944495764U, 1975308653U, 2586828582U, 2828430838U}},
  {{  24859044U,  748969300U, 2377960193U,  703249964U}}
};

fp_type CTEST_MANGLE_PREC2(expectedCombinedOperationsValue) = 39.036461095189096682;

#define CTEST_NO_ADVANCE


#include "checks.c.h"
#include "checks_prec.c.h"
