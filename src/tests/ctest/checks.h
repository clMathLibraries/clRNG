
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

#ifndef CTEST_ACTIONS_H
#define CTEST_ACTIONS_H

#include "mangle.h"
#include "util.h"

int CTEST_MANGLE(checkSuccessiveStates)();
//int CTEST_MANGLE_PREC(checkRandomU01,float)();
int CTEST_MANGLE_PREC(checkRandomU01,double)();
int CTEST_MANGLE_PREC(checkRandomInteger,double)();
int CTEST_MANGLE(checkCreateStreams)();
int CTEST_MANGLE(checkRewindStreamCreator)();
int CTEST_MANGLE(checkChangeStreamsSpacing)();
int CTEST_MANGLE(checkAdvanceStreams)();
int CTEST_MANGLE(checkRewindStreams)();
int CTEST_MANGLE(checkForwardToNextSubstreams)();
int CTEST_MANGLE(checkMakeSubstreams)();
int CTEST_MANGLE_PREC(checkCombinedOperations,float)();
int CTEST_MANGLE_PREC(checkCombinedOperations,double)();
int CTEST_MANGLE_PREC(checkDeviceOperations,float)(const DeviceSelect*);
int CTEST_MANGLE_PREC(checkDeviceOperations,double)(const DeviceSelect*);
int CTEST_MANGLE_PREC(checkDeviceRandomArray,float)(const DeviceSelect*);
int CTEST_MANGLE_PREC(checkDeviceRandomArray,double)(const DeviceSelect*);

#endif
