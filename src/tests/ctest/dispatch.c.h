
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

#include "mangle.h"
#include "checks.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

#include "../client/common.h"

#define _CTEST_WITH_NEW_STREAM(func, ...) \
  do { \
    clrngStatus _err; \
    clrngStream* _new_stream = clrngCreateStreams(NULL, 1, NULL, &_err); \
    check_error(_err, NULL); \
    func(_new_stream, __VA_ARGS__); \
    _err = clrngDestroyStreams(_new_stream); \
    check_error(_err, NULL); \
  } while (0)



int CTEST_MANGLE(dispatch)(const DeviceSelect* dev)
{
  int ret = 0;

  ret |= CTEST_MANGLE(checkSuccessiveStates)();
  ret |= CTEST_MANGLE_PREC(checkRandomU01,double)();
  ret |= CTEST_MANGLE(checkCreateStreams)();
  ret |= CTEST_MANGLE(checkRewindStreamCreator)();
  ret |= CTEST_MANGLE(checkChangeStreamsSpacing)();
  ret |= CTEST_MANGLE_PREC(checkRandomInteger,double)();
  ret |= CTEST_MANGLE(checkAdvanceStreams)();
  ret |= CTEST_MANGLE(checkRewindStreams)();
  ret |= CTEST_MANGLE(checkForwardToNextSubstreams)();
  ret |= CTEST_MANGLE(checkMakeSubstreams)();
  ret |= CTEST_MANGLE_PREC(checkCombinedOperations,float)();
  ret |= CTEST_MANGLE_PREC(checkCombinedOperations,double)();
  ret |= CTEST_MANGLE_PREC(checkDeviceOperations,float)(dev);
  ret |= CTEST_MANGLE_PREC(checkDeviceOperations,double)(dev);
  ret |= CTEST_MANGLE_PREC(checkDeviceRandomArray,float)(dev);
  ret |= CTEST_MANGLE_PREC(checkDeviceRandomArray,double)(dev);

  return ret;
}
