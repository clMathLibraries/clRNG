
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

/*! @file  anyrng.h
 *  @brief Link names of the generic API to a given RNG's API.
 *
 *  The purposose of this file is to facilitate testing the examples against
 *  different RNG implementations.
 *
 *  Exactly one of the preprocessor symbols \c USE_MRG31K3P, \c USE_MRG32K3A or
 *  \c USE_LFSR113 must be defined before including this header file.
 *
 *  The following preprocessor symbols are defined:
 *  - \c RNG_HOST_HEADER expands to the name of the RNG header file for the
 *    host code, surrounded with angle brackets;
 *  - \c RNG_DEVICE_HEADER expands to the name of the RNG header file for the
 *    device code, surrounded with angle brackets;
 *  - \c RNG_PREFIX expands to the RNG prefix that is normally expanded after
 *    the common \c clrng prefix, e.g., \c Mrg31k3p for the MRG31k3p RNG;
 *    host code, surrounded with angle brackets;
 *  - \c RNG_PREFIX_S and \c RNG_DEVICE_HEADER_S respectively expand to the
 *    values of \c RNG_PREFIX and \c RNG_DEVICE_HEADER, but surrounded with
 *    quotes (stringified);
 *  - \c RNG_DEVICE_HEADER expands to the name of the RNG header file for the
 *    device code, surrounded with angle brackets;
 *  - the clRNG API types and functions in a generic form, i.e., without the
 *    name of the RNG expanded after the \c clrng prefix.
 *
 *  To use the MRG31k3p generator, write:
 *
 *      #define USE_MRG31K3P
 *      #include <clRNGgen.h>
 */

#if defined(USE_MRG31K3P)   && !defined(USE_MRG32K3A) && !defined(USE_LFSR113)
    #define RNG_PREFIX      Mrg31k3p
    #define RNG_HEADER_NAME mrg31k3p
#elif defined(USE_MRG32K3A) && !defined(USE_LFSR113)
    #define RNG_PREFIX      Mrg32k3a
    #define RNG_HEADER_NAME mrg32k3a
#elif defined(USE_LFSR113)
    #define RNG_PREFIX      Lfsr113
    #define RNG_HEADER_NAME lfsr113
#else
    #error Exactly one of USE_MRG31K3P, USE_MRG32K3A or USE_LFSR113 must be defined.
#endif

#define RNG_HOST_HEADER   <RNG_HEADER_NAME.h>
#define RNG_DEVICE_HEADER <RNG_HEADER_NAME.clh>

#define RNG_MEMBER_(rng, ident)  RNG_MEMBER__(rng, ident)
#define RNG_MEMBER__(rng, ident) clrng ## rng ## ident

#define RNG_STR_(x)  RNG_STR__(x)
#define RNG_STR__(x) #x

#define RNG_PREFIX_S        RNG_STR_(RNG_PREFIX)
#define RNG_DEVICE_HEADER_S RNG_STR_(RNG_DEVICE_HEADER)


#include RNG_HOST_HEADER

#define clrngStreamState                RNG_MEMBER_(RNG_PREFIX, StreamState)
#define clrngStream                     RNG_MEMBER_(RNG_PREFIX, Stream)
#define clrngStreamCreator              RNG_MEMBER_(RNG_PREFIX, StreamCreator)
#define clrngCopyStreamCreator          RNG_MEMBER_(RNG_PREFIX, CopyStreamCreator)
#define clrngDestroyStreamCreator       RNG_MEMBER_(RNG_PREFIX, DestroyStreamCreator)
#define clrngSetBaseCreatorState        RNG_MEMBER_(RNG_PREFIX, SetBaseCreatorState)
#define clrngChangeStreamsSpacing       RNG_MEMBER_(RNG_PREFIX, ChangeStreamsSpacing)
#define clrngAllocStreams               RNG_MEMBER_(RNG_PREFIX, AllocStreams)
#define clrngDestroyStreams             RNG_MEMBER_(RNG_PREFIX, DestroyStreams)
#define clrngCreateStreams              RNG_MEMBER_(RNG_PREFIX, CreateStreams)
#define clrngCreateOverStreams          RNG_MEMBER_(RNG_PREFIX, CreateOverStreams)
#define clrngCopyStreams                RNG_MEMBER_(RNG_PREFIX, CopyStreams)
#define clrngCopyOverStreams            RNG_MEMBER_(RNG_PREFIX, CopyOverStreams)
#define clrngRandomU01                  RNG_MEMBER_(RNG_PREFIX, RandomU01)
#define clrngRandomInteger              RNG_MEMBER_(RNG_PREFIX, RandomInteger)
#define clrngRandomU01Array             RNG_MEMBER_(RNG_PREFIX, RandomU01Array)
#define clrngRandomIntegerArray         RNG_MEMBER_(RNG_PREFIX, RandomIntegerArray)
#define clrngRewindStreams              RNG_MEMBER_(RNG_PREFIX, RewindStreams)
#define clrngRewindSubstreams           RNG_MEMBER_(RNG_PREFIX, RewindSubstreams)
#define clrngForwardToNextSubstreams    RNG_MEMBER_(RNG_PREFIX, ForwardToNextSubstreams)
#define clrngMakeSubstreams             RNG_MEMBER_(RNG_PREFIX, MakeSubstreams)
#define clrngMakeOverSubstreams         RNG_MEMBER_(RNG_PREFIX, MakeOverSubstreams)
#define clrngAdvanceSubstreams          RNG_MEMBER_(RNG_PREFIX, AdvanceSubstreams)
#define clrngDeviceRandomU01Array       RNG_MEMBER_(RNG_PREFIX, DeviceRandomU01Array)
#define clrngWriteStreamInfo            RNG_MEMBER_(RNG_PREFIX, WriteStreamInfo)

#ifdef __CLRNG_DEVICE_API
#define clrngCopyOverStreamsFromGlobal    RNG_MEMBER_(RNG_PREFIX, CopyOverStreamsFromGlobal)
#define clrngCopyOverStreamsToGlobal      RNG_MEMBER_(RNG_PREFIX, CopyOverStreamsToGlobal)
#endif

