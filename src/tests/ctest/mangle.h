
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

#pragma once
#ifndef CTEST_COMMON_H
#define CTEST_COMMON_H

#ifdef __APPLE__
#include <Opencl/cl.h>
#else
#include <CL/cl.h>
#endif


#ifndef CTEST_RNG_TYPE
#error "CTEST_RNG_TYPE undefined"
#endif

#ifndef CTEST_RNG_HEADER
#error "CTEST_RNG_HEADER undefined"
#endif

#define RNG_HOST_HEADER   <CTEST_RNG_HEADER.h>
#define RNG_DEVICE_HEADER <CTEST_RNG_HEADER.clh>

#define _RNG_MANGLE(ident)          _RNG_MANGLE_(ident,CTEST_RNG_TYPE)
#define _RNG_MANGLE_(ident,rng)     _RNG_MANGLE__(ident,rng)
#define _RNG_MANGLE__(ident,rng)    clrng ## rng ## ident

#define CTEST_MANGLE(ident)        CTEST_MANGLE_(ident,CTEST_RNG_TYPE)
#define CTEST_MANGLE_(ident,rng)   CTEST_MANGLE__(ident,rng)
#define CTEST_MANGLE__(ident,rng)  ctest ## rng ## _ ## ident

#define CTEST_MANGLE_PREC(ident,fp)        CTEST_MANGLE_PREC_(ident,CTEST_RNG_TYPE,fp)
#define CTEST_MANGLE_PREC_(ident,rng,fp)   CTEST_MANGLE_PREC__(ident,rng,fp)
#define CTEST_MANGLE_PREC__(ident,rng,fp)  ctest ## rng ## _ ## fp ## _ ## ident

#ifdef CLRNG_SINGLE_PRECISION
#define CTEST_MANGLE_PREC2(ident)   CTEST_MANGLE_PREC_(ident,CTEST_RNG_TYPE,float)
#define write_array		    write_array_cl_float
typedef cl_float fp_type;
#define DEVICE_FP_TYPE		    "float"
#else
#define CTEST_MANGLE_PREC2(ident)  CTEST_MANGLE_PREC_(ident,CTEST_RNG_TYPE,double)
#define write_array		    write_array_cl_double
typedef cl_double fp_type;
#define DEVICE_FP_TYPE		    "double"
#endif

// utility macro
#define CTEST_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// for size types
#if defined(_MSC_VER) || defined(__MINGW32__)
    #define SIZE_T_FORMAT   "Iu"
#elif defined(__GNUC__)
    #define SIZE_T_FORMAT   "zu"
#else
    #define SIZE_T_FORMAT   "lu"
#endif

#define _CTEST_STR_(x)  _CTEST_STR__(x)
#define _CTEST_STR__(x) #x

#define RNG_TYPE_S          _CTEST_STR_(CTEST_RNG_TYPE)
#define RNG_DEVICE_HEADER_S _CTEST_STR_(RNG_DEVICE_HEADER)

#define clrngStreamState                _RNG_MANGLE(StreamState)
#define clrngStream                     _RNG_MANGLE(Stream)
#define clrngStreamCreator              _RNG_MANGLE(StreamCreator)
#define clrngCopyStreamCreator          _RNG_MANGLE(CopyStreamCreator)
#define clrngDestroyStreamCreator       _RNG_MANGLE(DestroyStreamCreator)
#define clrngRewindStreamCreator        _RNG_MANGLE(RewindStreamCreator)
#define clrngSetBaseCreatorState        _RNG_MANGLE(SetBaseCreatorState)
#define clrngChangeStreamsSpacing       _RNG_MANGLE(ChangeStreamsSpacing)
#define clrngAllocStreams               _RNG_MANGLE(AllocStreams)
#define clrngDestroyStreams             _RNG_MANGLE(DestroyStreams)
#define clrngCreateStreams              _RNG_MANGLE(CreateStreams)
#define clrngCreateOverStreams          _RNG_MANGLE(CreateOverStreams)
#define clrngCopyStreams                _RNG_MANGLE(CopyStreams)
#define clrngCopyOverStreams            _RNG_MANGLE(CopyOverStreams)
#define clrngRandomU01                  _RNG_MANGLE(RandomU01)
#define clrngRandomInteger              _RNG_MANGLE(RandomInteger)
#define clrngRandomU01Array             _RNG_MANGLE(RandomU01Array)
#define clrngRandomIntegerArray         _RNG_MANGLE(RandomIntegerArray)
#define clrngRewindStreams              _RNG_MANGLE(RewindStreams)
#define clrngRewindSubstreams           _RNG_MANGLE(RewindSubstreams)
#define clrngForwardToNextSubstreams    _RNG_MANGLE(ForwardToNextSubstreams)
#define clrngMakeSubstreams             _RNG_MANGLE(MakeSubstreams)
#define clrngMakeOverSubstreams         _RNG_MANGLE(MakeOverSubstreams)
#define clrngAdvanceStreams             _RNG_MANGLE(AdvanceStreams)
#define clrngDeviceRandomU01Array       _RNG_MANGLE(DeviceRandomU01Array)
#define clrngWriteStreamInfo            _RNG_MANGLE(WriteStreamInfo)

#include RNG_HOST_HEADER

// defined in ctest_cli.c
extern int ctestVerbose;

#endif
