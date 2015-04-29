
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
#ifndef CLRNG_TEMPLATE_H
#define CLRNG_TEMPLATE_H

#error This file is a template for specific RNG implementations; it should not be included as is.  Use an RNG-specific header such as mrg31k3p.h instead.

#include <clRNG.h>

/* for FILE in clrngWriteStreamInfo() */
#include <stdio.h>


/*! @file clRNG_template.h
 *  @brief Template of the specialized interface for specific generators (not to be included as is!)
 *
 *  The function and type names in this API all start with \c clrng.
 *  In each specific implementation, this prefix is expanded to a specific prefix;
 *  e.g., \c clrngMrg31k3p for the MRG31k3p generator.
 *  
 *  In the standard case, streams and substreams are defined as in \cite iLEC08j,
 *  \cite rLEC91a, \cite rLEC02a .
 *  The sequence of successive states of the base RNG over its entire period of length \f$\rho\f$ 
 *  is divided into streams whose starting points are \f$Z\f$ steps apart.
 *  The sequence for each stream (of length \f$Z\f$) is further divided into substreams of length \f$W\f$. 
 *  The integers \f$Z\f$ and \f$W\f$ have default values that have been carefully selected 
 *  to avoid detectable dependence between successive streams and substreams, 
 *  and are large enough to make sure that streams and substreams will not be exhausted in practice.
 *  It is strongly recommended to never change these values (even if the software allows it).
 *  The initial state of the first stream (the seed of the library) has a default value.
 *  It can be changed by invoking clrngSetBaseCreatorState() before creating a first stream.
 *  
 *  A stream object is a structure that contains the current state of the stream, 
 *  its initial state (at the beginning of the stream),
 *  and the initial state of the current substream.
 *  Whenever the user creates a new stream, the software automatically jumps ahead by \f$Z\f$ steps 
 *  to find its initial state, and the three states in the stream object are set to it.
 *  The form of the state depends on the type of RNG.
 *  
 *  Some functions are available on both the host and the devices 
 *  (they can be used  within a kernel) whereas others (such as stream creation) 
 *  are available only on the host.
 *  Many functions are defined only for arrays of streams; 
 *  for a single stream, it suffices to specify an array of size 1.
 *  
 *  When a kernel is called, one should pass a copy of the streams from the host 
 *  to the global memory of the device.
 *  Another copy of the stream state must be stored
 *  in the private memory of the work item that uses it in the kernel code to generate random numbers. 
 *  In the current implementation, to avoid wasting (scarce) private memory,
 *  without the option \c CLRNG_ENABLE_SUBSTREAMS (see below),
 *  only the current state of the stream is stored explicitly in the work-item's private memory.
 *  The work item also keeps in private memory a pointer to the initial state 
 *  of the stream, but this initial state is not copied into private memory,
 *  and the work item does not keep track of the initial state of the current substream.
 *  With the option  \c CLRNG_ENABLE_SUBSTREAMS,
 *  the initial state of the current substream is also stored into private memory.
 *  This permits one to rewind the current state to it or move forward to the next substream.
 *  
 *  To use the clRNG library from within a user-defined kernel, the
 *  user must include the clRNG header file corresponding to the desired RNG
 *  via an \c include directive.
 *  Other specific preprocessor macros can be placed *before*
 *  including the header file
 *  to change settings of the library when the default values are not suitable for the user.
 *  The following options are currently available:
 *
 *  - \c CLRNG_SINGLE_PRECISION:
 *    With this option, all the random numbers returned by clrngRandomU01() and
 *    clrngRandomU01Array(), and generated by clrngDeviceRandomU01Array(), will be of type
 *    \c cl_float instead of \c cl_double (the default setting).
 *    This option can be activated on the device and the host separately (i.e., on
 *    either one or on both), and affects all implemented RNGs.
 *  - \c CLRNG_ENABLE_SUBSTREAMS:
 *    With this option, the current state of a stream can be reset on the device to the initial state of the
 *    current or next substream. This is made possible by storing in private memory
 *    the initial substream state.
 *    Without this option, by default, this is not possible and only the current
 *    state of the stream and a pointer to its initial state (left in global memory)
 *    are kept in private memory and is accessible, in a work item.
 *    **This option applies only to the device; operations on substreams are 
 *    always available on the host.**
 *
 *  For example, to enable substreams support, generate single-precision floating point numbers 
 *  on the device, and use the MRG31k3p generator, one would have, in the device
 *  code:
 *  \code{c}
 *      #define  CLRNG_ENABLE_SUBSTREAMS
 *      #define  CLRNG_SINGLE_PRECISION
 *      #include <mrg31k3p.clh>
 *  \endcode
 *
 *  To generate single-precision floating point numbers also on the host, still
 *  using the MRG31k3p generator, the host code should contain:
 *  \code{c}
 *      #define CLRNG_SINGLE_PRECISION
 *      #include <mrg31k3p.h>
 *  \endcode
 *  
 *  The functions described here are all available on the host, in all implementations, 
 *  unless specified otherwise.  Only some of the functions and types are also
 *  available on the device in addition to the host;  they are tagged with
 *  [**device**].
 *  Other functions are only available on the device; they are tagged with
 *  [**device-only**].
 *  Some functions return an error code in \c err.
 *
 *
 *  # Implemented RNG's
 *
 *  The following table lists the RNG's that are currently implemented in clRNG
 *  with the name of the corresponding header file.
 *
 *  | RNG            | Prefix        | Host Header File | Device Header File |
 *  | -------------- | ------------- | ---------------- | ------------------ |
 *  | MRG31k3p       | `Mrg31k3p`    | mrg31k3p.h       | mrg31k3p.clh       |
 *  | MRG32k3a       | `Mrg32k3a`    | mrg32k3a.h       | mrg32k3a.clh       |
 *  | LFSR113        | `Lfsr113`     | lfsr113.h        | lfsr113.clh        |
 *  | Philox-4×32-10 | `Philox432`   | philox432.h      | philox432.clh      |
 *
 *
 *  ## The MRG31k3p Generator
 *
 *  The MRG31k3p generator is defined in \cite rLEC00b .
 *  In its specific implementation, the function and type names start with \c clrngMrg31k3p.
 *  For this RNG, a *state* is a vector of six 31-bit integers, 
 *  represented internally as \c cl_uint.
 *  The entire period length of approximately \f$2^{185}\f$ is divided into 
 *  approximately \f$2^{51}\f$ non-overlapping streams of length \f$Z=2^{134}\f$.
 *  Each stream is further partitioned into substreams of length \f$W=2^{72}\f$.
 *  The state (and seed) of each stream is a vector of six 31-bit integers.
 *  This size of state is appropriate for having streams running in work items on GPU cards,
 *  for example, while providing a sufficient period length for most applications. 
 *
 *
 *  ## The MRG32k3a Generator
 *
 *  MRG32k3a is a combined multiple recursive generator (MRG) proposed by L'Ecuyer \cite rLEC99b ,
 *  implemented here in 64-bit integer arithmetic.
 *  This RNG has a period length of approximately \f$2^{191}\f$,
 *  and is divided into approximately \f$2^{64}\f$ non-overlapping streams of length \f$Z = 2^{127}\f$,
 *  and each stream is subdivided in \f$2^{51}\f$ substreams of length \f$W = 2^{76}\f$.
 *  These are the same numbers as in \cite rLEC02a .
 *  The state of a stream at any given step is a six-dimensional vector of 32-bit integers, 
 *  but those integers are stored as \c cl_ulong (64-bit integers) in the present implementation 
 *  (so they use twice the space).
 *  The generator has 32 bits of resolution.
 *  Note that in the original version proposed in \cite rLEC99b and \cite rLEC02a, 
 *  the recurrences are implemented in \c double instead,
 *  and the state is stored in six 32-bit integers.
 *  The change in implementation is to avoid using \c double's,
 *  which are not available on many GPU devices, and also because the 64-bit implementation
 *  is much faster than that in \c double when 64-bit integer arithmetic is available on the hardware.
 *
 *
 *  ## The LFSR113 Generator
 *
 *  The LFSR113 generator is defined in \cite rLEC99a .
 *  In its implementation, the function and type names start with \c clrngLfsr113.
 *  For this RNG, a *state* vector of four 31-bit integers, 
 *  represented internally as \c cl_uint.
 *  The period length of approximately \f$2^{113}\f$ is divided into 
 *  approximately \f$2^{23}\f$ non-overlapping streams of length \f$Z=2^{90}\f$.
 *  Each stream is further partitioned into \f$2^{35}\f$ substreams of length \f$W=2^{55}\f$.
 *  Note that the functions \c clrngLfsr113ChangeStreamsSpacing() and 
 *  \c clrngLfsr113AdvancedStreams() are not implemented in the current version.
 *
 *
 *  ## The Philox-4×32-10 Generator
 *
 *  The counter-based Philox-4×32-10 generator is defined in \cite rSAL11a .
 *  Unlike the previous three generators, its design is not supported by a 
 *  theoretical analysis of equidistribution.
 *  It has only been subjected to empirical testing with the TestU01 software 
 *  \cite rLEC07b (the other three generators also have).
 *  In its implementation, the function and type
 *  names start with \c clrngPhilox432.
 *  For this RNG, a *state* is a 128-bit counter with a 64-bit key, 
 *  and a 2-bit index used to iterate over the four 32-bit outputs generated for 
 *  each counter value.
 *  The counter is represented internally as a vector of four 32-bit \c cl_uint
 *  values and the index, as a single \c cl_uint value.
 *  In the current clRNG version, the key is the same for all streams, so it 
 *  is not stored in each stream object but rather hardcoded in the implementation.
 *  The period length of \f$2^{130}\f$ is divided into \f$2^{28}\f$ non-overlapping streams 
 *  of length \f$Z=2^{102}\f$.
 *  Each stream is further partitioned into \f$2^{36}\f$ substreams of length \f$W=2^{66}\f$.
 *  The key (all bits to 0), initial counter and order in which the four outputs 
 *  per counter value are returned are chosen to generate the same values, in the 
 *  same order, as Random123's \c Engine module \cite rSAL11a, designed for 
 *  use with the standard C++11 \c random library.
 *  Note that the function \c clrngPhilox432ChangeStreamsSpacing() supports 
 *  only values of \f$c\f$ that are multiples of 4, with either \f$e = 0\f$ or \f$e 
 *  \geq 2\f$.
 *  
 */



/*! @brief Stream state [**device**]
 *
 *  Contains the state of a random stream.
 *  The definition of a state depends on the type of generator.
 */
typedef struct { /* ... */ } clrngStreamState;


/*! @brief Stream object [**device**]
 *
 *  A structure that contains the current information on a stream object.  
 *  It generally depends on the type of generator.
 *  It typically stores the current state, the initial state of the stream, 
 *  and the initial state of the current substream.
 *
 *  @note The device API offers a variant of this struct definition called
 *  \c clrngHostStream to receive stream objects from the host.  Stream objects,
 *  as defined on the device, do not store as much information as stream
 *  objects on the host, but keep pointers to relevant information from the
 *  host stream object.
 *  The definition of the clrngStream type on the device also 
 *  depends on whether substreams support is required by the user (with the 
 *  \c CLRNG_ENABLE_SUBSTREAMS option).
 */
typedef struct { /* ... */ } clrngStream;


/*! @brief Host stream object [**device-only**]
 *
 *  Variant of clrngStream that represents stream objects received from the host by the
 *  device.  These are normally stored in global memory.
 */
typedef struct { /* ... */ } clrngHostStream;


/*! @brief Stream creator object
 *
 *  For each type of RNG, there is a single default creator of streams, and this should
 *  be sufficient for most applications. 
 *  Multiple creators could be useful for example to create the same successive 
 *  stream objects multiple times in the same order, instead of storing them in an 
 *  array and reusing them, or to create copies of the same streams in the same order 
 *  at different locations in a distributed system, e.g., when simulating similar systems 
 *  with common random numbers. 
 *  Stream creators are created according to an abstract factory pattern.
 */
typedef struct { /* ... */ } clrngStreamCreator;


#ifdef __cplusplus
extern "C" {
#endif


/*! @name Stream Creators
 *
 *  Functions to create, destroy and modify stream creator objects (factory pattern).
 */
/*! @{ */


/*! @brief Duplicate an existing stream creator object
 *
 *  Create an identical copy (a clone) of the stream creator 
 *  \c creator.
 *  To create a copy of the default creator, put \c NULL as the 
 *  \c creator parameter.
 *  All the new stream creators returned by `clrngCopyStreamCreator(NULL, 
 *   NULL)` will create the same sequence of random streams,
 *  unless the default stream creator is used to create streams between 
 *  successive calls to this function.
 *
 *  @param[in]  creator     Stream creator object to be copied, or `NULL` to
 *                          copy the default stream creator.
 *  @param[out] err         Error status variable, or `NULL`.
 *  @return     The newly created stream creator object.
 */
clrngStreamCreator* clrngCopyStreamCreator(const clrngStreamCreator* creator, clrngStatus* err);


/*! @brief Destroy a stream creator object
 *
 *  Release the resources associated to a stream creator object.
 *
 *  @param[out] creator     Stream creator object to be destroyed.
 *  @return     Error status
 */
clrngStatus clrngDestroyStreamCreator(clrngStreamCreator* creator);

/*! @brief Reset a stream creator to its original initial state
 *
 *  Reset the stream creator to its original initial state, so it can
 *  re-create the same streams over again.
 *
 *  @param[in]  creator     Stream creator object to be reset.
 *  @return     Error status
 */
clrngStatus clrngRewindStreamCreator(clrngStreamCreator* creator);

/*! @brief Change the base stream state of a stream creator
 *
 *  Set the base state of the stream creator, which can be seen as the seed of the underlying RNG.
 *  This will be the initial state (or seed) of the first stream created by this creator.
 *  Then, for most conventional RNGs, the initial states of successive streams will be spaced equally, 
 *  by \f$Z\f$ steps in the RNG sequence.
 *  The type and size of the \c baseState parameter depends on the type of RNG.
 *  The base state always has a default value, so this function does not need to be invoked.
 *
 *  @param[in,out]  creator     Stream creator object.
 *  @param[in]      baseState   New initial base stream state.  Can be set to
 *                              `NULL` to use the library default.
 *  @return     Error status
 *
 *  @warning It is recommended to use the library default base state.
 */
clrngStatus clrngSetBaseCreatorState(clrngStreamCreator* creator, const clrngStreamState* baseState);


/*! @brief Change the spacing between successive streams
 *
 *  **This function should be used only in exceptional circumstances.**
 *  It changes the spacing \f$Z\f$ between the initial states of the successive streams
 *  from the default value to \f$Z = 2^e + c\f$ if \f$e > 0\f$, or to
 *  \f$Z = c\f$ if \f$e = 0\f$.
 *  One must have \f$e \geq 0\f$ but \f$c\f$ can take negative values.
 *  The default spacing values have been carefully selected for each RNG 
 *  to avoid overlap and dependence between streams, and it is highly recommended 
 *  not to change them.
 *
 *  @param[in,out]  creator     Stream creator object.
 *  @param[in]      e           Value of \f$e\f$.
 *  @param[in]      c           Value of \f$c\f$.
 *  @return     Error status
 *
 *  @warning It is recommended to use the library default spacing and not to invoke
 *  this function.
 */
clrngStatus clrngChangeStreamsSpacing(clrngStreamCreator* creator, cl_int e, cl_int c);


/*! @} */


/*! @name Stream Allocation, Destruction and Initialization
 *
 *  Functions to create or destroy random streams and arrays of random
 *  streams.
 */
/*! @{ */


/*! @brief Reserve memory for one or more stream objects
 *
 *  Reserve memory space for \c count stream objects, without creating the 
 *  stream objects.  Returns a pointer to the allocated buffer and returns 
 *  in \c bufSize the size of the allocated buffer, in bytes.
 *
 *  @param[in]      count       Number of stream objects to allocate.
 *  @param[out]     bufSize     Size in bytes of the allocated buffer, or
 *                              `NULL` if not needed.
 *  @param[out]     err         Error status variable, or `NULL`.
 *  @return     Pointer to the newly allocated buffer.
 */
clrngStream* clrngAllocStreams(size_t count, size_t* bufSize, clrngStatus* err);


/*! @brief Destroy one or many stream objects
 *
 *  Release the memory space taken by those stream objects.
 *
 *  @param[in,out]  streams      Stream object buffer to be released.
 *  @return     Error status
 */
clrngStatus clrngDestroyStreams(clrngStream* streams);


/*! @brief Allocate memory for and create new RNG stream objects
 *
 *  Create and return an array of \c count new streams using the specified creator.
 *  This function also reserves the memory space required for the structures 
 *  and initializes the stream states.
 *  It returns in \c bufSize the size of the allocated buffer, in bytes.
 *  To use the default creator, put \c NULL as the \c creator parameter. 
 *  To create a single stream, just put set \c count to 1.
 *
 *  @param[in,out]  creator     Stream creator object, or `NULL` to use the
 *                              default stream creator.
 *  @param[in]      count       Size of the array (use 1 for a single stream
 *                              object).
 *  @param[out]     bufSize     Size in bytes of the allocated buffer, or
 *                              `NULL` if not needed.
 *  @param[out]     err         Error status variable, or `NULL`.
 *  @return     The newly created array of stream object.
 */
clrngStream* clrngCreateStreams(clrngStreamCreator* creator, size_t count, size_t* bufSize, clrngStatus* err);


/*! @brief Create new RNG stream objects in already allocated memory
 *
 *  This function is similar to \c clrngCreateStreams(), except that it does not reserve 
 *  memory for the structure.  
 *  It creates the array of new streams in the preallocated \c streams 
 *  buffer,
 *  which could have been reserved earlier via either \c clrngAllocStreams() 
 *  or \c clrngCreateStreams(). It permits the client to reuse memory that was previously
 *  allocated for other streams.
 *
 *  @param[in,out]  creator     Stream creator object, or `NULL` to use the
 *                              default stream creator.
 *  @param[in]      count       Size of the array (use 1 for a single stream
 *                              object).
 *  @param[out]     streams     Buffer in which the new stream(s) will be stored.
 *
 *  @return     Error status
 */
clrngStatus clrngCreateOverStreams(clrngStreamCreator* creator, size_t count, clrngStream* streams);


/*! @brief Clone RNG stream objects
 *
 *  Create an identical copy (a clone) of each of the \c count stream objects 
 *  in the array \c streams.
 *  This function allocates memory for all the new structures before cloning, and returns 
 *  a pointer to the new structure. 
 *
  *  @param[in]  count       Number of random number in the array (use 1 for a
 *                          single stream object).
 *  @param[in]  streams     Stream object or array of stream objects to be cloned.
 *
 *  @param[out] err         Error status variable, or `NULL`.
 *  @return     The newly created stream object or array of stream objects.
 */
clrngStream* clrngCopyStreams(size_t count, const clrngStream* streams, clrngStatus* err);


/*! @brief Copy RNG stream objects in already allocated memory [**device**]
 *
 *  Copy (or restore) the stream objects \c srcStreams into the buffer \c destStreams, 
 *  and each of the \c count stream objects from the array 
 *  \c srcStreams 
 *  into the buffer \c destStreams. 
 *  This function *does not* allocate memory for the structures in \c destStreams; 
 *  it assumes that this has already been done.
 *  Note: The device API offers variants of this function
 *  to convert stream objects across their host and device 
 *   representations, while copying across different types of memory.
 *
 *  @param[in]  count			Number of stream objects to copy (use 1 for a single
 *								stream object).
 *  @param[out] destStreams		Destination buffer into which to copy (its
 *								content will be overwritten).
 *  @param[in]  srcStreams		Stream object or array of stream objects to be copied.
 *
 *  @return     Error status
 *
 *  @note The device API offers two variants of this function called
 *  clrngCopyOverStreamsFromGlobal(), for which \c srcStreams is a pointer to
 *  constant clrngHostStream objects in global memory, and
 *  clrngCopyOverStreamsToGlobal(), for which \c destStreams is a pointer to
 *  (non-constant) clrngHostStream objects in global memory.
 */
clrngStatus clrngCopyOverStreams(size_t count, clrngStream* destStreams, const clrngStream* srcStreams);


/*! @brief Copy RNG host stream objects from global memory into private memory [**device-only**]
 *
 *  Copy the host stream objects \c srcStreams from global memory as device
 *  stream objects into the buffer \c destStreams in private memory.
 *
 *  @param[in]  count			Number of stream objects to copy (use 1 for a single
 *								stream object).
 *  @param[out] destStreams		Destination buffer into which to copy (its
 *								content will be overwritten).
 *  @param[in]  srcStreams		Host stream object or array of host stream objects to be copied.
 *
 *  @return     Error status
 */
clrngStatus clrngCopyOverStreamsFromGlobal(size_t count, clrngStream* destStreams, const clrngHostStream* srcStreams);


/*! @brief Copy RNG device stream objects from private memory into global memory [**device-only**]
 *
 *  @param[in]  count			Number of stream objects to copy (use 1 for a single
 *								stream object).
 *  @param[out] destStreams		Destination buffer into which to copy (its
 *								content will be overwritten).
 *  @param[in]  srcStreams		Device stream object or array of device stream objects to be copied.
 *
 *  @return     Error status
 */
clrngStatus clrngCopyOverStreamsToGlobal(size_t count, clrngHostStream* destStreams, const clrngStream* srcStreams);


/*! @} */


/*! @name Stream Output
 *
 *  Functions to read successive values from a random stream.
 */
/*! @{ */


/*! @brief Generate the next random value in (0,1) [**device**]
 *
 *  Generate and return a (pseudo)random number from the uniform distribution over the
 *  interval \f$(0,1)\f$, using \c stream.
 *  If this stream is from an RNG, the stream state is advanced by one step before producing the
 *  (pseudo)random number.
 *
 *  By default, the returned value is of type \c cl_double.
 *  But if the option \c CLRNG_SINGLE_PRECISION is defined, 
 *  the returned value will be of type \c cl_float.  
 *  Setting this option changes the type of the returned value for all RNGs
 *  and all functions that use clrngRandomU01().
 *
 *  @param[in,out]  stream      Stream used to generate the random value.
 *  @return     A random floating-point value uniformly distributed in
 *              \f$(0,1)\f$
 */
cl_double clrngRandomU01(clrngStream* stream);


/*! @brief Generate the next random integer value [**device**]
 *  
 *  Generate and return a (pseudo)random integer from the discrete uniform distribution 
 *  over the integers \f$\{i, \dots, j \}\f$, using \c stream, by calling
 *  clrngRandomU01() once and transforming the output by inversion.
 *  That is, it returns `i + (cl_int)((j-i+1) * clrngRandomU01(stream))`.
 *
 *  @param[in,out]  stream      Stream used to generate the random value.
 *  @param[in]      i           Smallest integer value (inclusive).
 *  @param[in]      j           Largest integer value (inclusive).
 *  @return     A random integer value uniformly distributed in \f$\{i, \dots, j \}\f$
 */
cl_int clrngRandomInteger(clrngStream* stream, cl_int i, cl_int j);


/*! @brief Fill an array with successive random values in (0,1) [**device**]
 *
 *  Fill preallocated \c buffer with \c count successive (pseudo)random numbers.
 *  Equivalent to calling clrngRandomU01() \c count times to fill the buffer.
 *
 *  If \c CLRNG_SINGLE_PRECISION is defined, the buffer argument is of type \c
 *  cl_float and will be filled by \c count values of type \c cl_float instead.
 *
 *  @param[in,out]  stream      Stream used to generate the random values.
 *  @param[in]      count       Number of values in the array.
 *  @param[out]     buffer      Destination buffer (must be pre-allocated).
 *  @return     Error status
 */
clrngStatus clrngRandomU01Array(clrngStream* stream, size_t count, cl_double* buffer);


/*! @brief Fill an array with successive random integer values [**device**]
 *
 *  Same as clrngRandomU01Array(), but for integer values in \f$\{i,\dots,j\}\f$.
 *  Equivalent to calling clrngRandomInteger() \c count times to fill the buffer.
 *
 *  @param[in,out]  stream      Stream used to generate the random values.
 *  @param[in]      i           Smallest integer value (inclusive).
 *  @param[in]      j           Largest integer value (inclusive).
 *  @param[in]      count       Number of values in the array.
 *  @param[out]     buffer      Destination buffer (must be pre-allocated).
 *  @return     Error status
 */
clrngStatus clrngRandomIntegerArray(clrngStream* stream, cl_int i, cl_int j, size_t count, cl_int* buffer);


/*! @name Stream Navigation
 *
 *  Functions to roll back or advance streams by many steps.
 */
/*! @{ */


/*! @brief Reinitialize streams to their initial states [**device**]
 *
 *  Reinitialize all the streams in \c streams to their initial states.
 *  The current substream also becomes the initial one.
 *
 *  @param[in]      count       Number of stream objects in the array (use 1
 *                              for a single stream object).
 *  @param[in,out]  streams     Stream object or array of stream objects to be
 *                              reset to the start of the stream(s).
 *  @return     Error status
 *
 *  @warning This function can be slow on the device, because it reads the
 *  initial state from global memory.
 */
clrngStatus clrngRewindStreams(size_t count, clrngStream* streams);


/*! @brief Reinitialize streams to their initial substream states [**device**]
 *
 *  Reinitialize all the streams in \c streams to the initial states of
 *  their current substream.
 *
 *  @param[in]      count       Number of stream objects in the array (use 1
 *                              for a single stream object).
 *  @param[in,out]  streams     Stream object or array of stream objects to be
 *                              reset to the beginning of the current substream(s).
 *  @return     Error status
 *
 *  @warning This function is available on the device only if the preprocessor
 *  symbol \c CLRNG_ENABLE_SUBSTREAMS is defined.
 */
clrngStatus clrngRewindSubstreams(size_t count, clrngStream* streams);


/*! @brief Advance streams to the next substreams [**device**]
 *
 *  Reinitialize all the streams in \c streams to the initial states of
 *  their next substream. The current states and the initial states of the current substreams are changed.
 *
 *  @param[in]      count       Number of stream objects in the array (use 1
 *                              for a single stream object).
 *  @param[in,out]  streams     Stream object or array of stream objects to be
 *                              advanced to the next substream(s).
 *  @return     Error status
 *
 *  @warning This function is available on the device only if the preprocessor
 *  symbol CLRNG_ENABLE_SUBSTREAMS is defined.
 */
clrngStatus clrngForwardToNextSubstreams(size_t count, clrngStream* streams);


/*! @brief Allocate and make an array of substreams of a stream
 *
 * Make and return an array of \c count copies of \c stream, whose current (and
 * initial substream) states are the initial states of \c count successive
 * substreams of \c stream.
 * The first substream in the returned array is simply a copy of \c stream.
 * This function also reserves the memory space required for the structures and
 * initializes the stream states.
 * It returns in \c bufSize the size of the allocated buffer, in bytes.
 * To create a single stream, just set \c count to 1.
 * When this function is invoked, the substream state and initial state of
 * \c stream are advanced by \c count substreams.
 */
clrngStream* clrngMakeSubstreams(clrngStream* stream, size_t count, size_t* bufSize, clrngStatus* err);


/*! @brief Make an array of substreams of a stream
 *
 * This function is similar to clrngMakeStreams(), except that it does not reserve
 * memory for the structure.
 * It creates the array of new streams in the preallocated \c substreams
 * buffer, which could have been reserved earlier via either
 * clrngAllocStreams(), clrngMakeSubstreams() or clrngCreateStreams(). It
 * permits the client to reuse memory that was previously allocated for other
 * streams.
 */
clrngStatus clrngMakeOverSubstreams(clrngStream* stream, size_t count, clrngStream* substreams);


/*! @brief Advance the state of streams by many steps
 *
 *  **This function should be used only in very exceptional circumstances.**
 *  It advances the state of the streams in array \c streams by \f$k\f$ steps, 
 *  without modifying the states of other streams, 
 *  nor the initial stream and substream states for those streams. 
 *  If \f$e > 0\f$, then \f$k = 2^e + c\f$;
 *  if \f$e < 0\f$, then \f$k = −2^{|e|} + c\f$; and if \f$e = 0\f$, then \f$k = c\f$.
 *  Note that \f$c\f$ can take negative values.
 *  We discourage the use of this procedure to customize the length of streams and substreams.
 *  It is better to use the default spacing, which has been carefully selected for each RNG type.
 *
 *  @param[in]      count       Number of stream objects in the array (use 1
 *                              for a single stream object).
 *  @param[in,out]  streams     Stream object or array of stream objects to be
 *                              advanced.
 *  @param[in]      e           Value of \f$e\f$.
 *  @param[in]      c           Value of \f$c\f$.
 *  @return     Error status
 *
 *  @warning Check the implementation for all cases \f$e > 0\f$, \f$e = 0\f$
 *  and \f$e < 0\f$.
 */
clrngStatus clrngAdvanceStreams(size_t count, clrngStream* streams, cl_int e, cl_int c);


/*! @} */


/*! @name Work Functions
 *
 *  Functions to submit work-items to command queues.
 */
/*! @{ */

/*! @brief      Fill a buffer of random numbers.
 *
 *  Fill the buffer pointed to by \c outBuffer with \c numberCount 
 *  uniform random numbers
 *  of type \c cl_double (or of type \c cl_float if \c CLRNG_SINGLE_PRECISION
 *  is defined), using \c streamCount work items.
 *  In the current implementation, \c numberCount must be a multiple of \c streamCount.
 *  See clEnqueueNDRangeKernel() from the OpenCL API documentation for a
 *  description of the \c numWaitEvents and
 *  \c waitEvents arguments.
 *
 *  @param[in]  streamCount            Number of streams in \c stream_array.
 *  @param[in]  streams                 OpenCL buffer that contains an array of
 *                                      stream objects.
 *  @param[in]  numberCount             Number of random number to store in the buffer.
 *  @param[out] outBuffer               OpenCL buffer in which the generated
 *                                      numbers will be stored.
 *  @param[in]  numQueuesAndEvents      numbers of queues and events (for now only the value 1 is supported)
 *  @param[in]  commQueues              OpenCL command queue.
 *  @param[in]  numWaitEvents           Number of events in the event wait list.
 *  @param[in]  waitEvents              Event wait list.
 *  @param[in]  outEvents                   Event object.
 *  @return     Error status
 *
 *  @note This function requires access to the clRNG device 
 *  header files (like \c mrg31k3p.clh) and assumes that the environment 
 *  variable \c CLRNG_ROOT points to the installation path of the 
 *  clRNG package, where lies the \c cl/include subdirectory 
 *  that contains these files.
 *  Means of setting an environment variable depend on the operating system 
 *  used.
 *
 *  @todo Add instructions for OSX, Linux and Windows users on how to set this
 *  environment variable.
 *
 *  @warning In the current implementation, \c numberCount must be a multiple
 *  of \c streamCount.
 *
 *  @warning In the current implementation, the array \c streams is left
 *  unchanged, as there is no write-back from the device code.
 *
 *  @warning In the current implementation, if the device doesn't support OpenCL
 *  2.0, \c numberCount must be smaller than or a multiple of the device's
 *  maximum workgroup size.
 *
 *  @warning In the current implementation, only \c numQueuesAndEvents = 1 is
 *  supported.
 *
 *  Assume the variables \c context and \c queue are a valid OpenCL context and
 *  command queue, and that we want to generate \c number_count using \c
 *  stream_count distinct streams, where \c number_count is a multiple of \c
 *  stream_count.
 *  To use the clrngDeviceRandomU01Array() function, we must first create
 *  an array of stream objects:
 *  \snippet RandomArray/randomarray.c streams allocation
 *  and create the OpenCL buffers for the stream objects:
 *  \snippet RandomArray/randomarray.c streams buffer
 *  and for the generated numbers:
 *  \snippet RandomArray/randomarray.c numbers buffer
 *  Next, these can be used to invoke clrngDeviceRandomU01Array():
 *  \snippet RandomArray/randomarray.c random array call
 *  At the moment the generated numbers are needed, we must wait for the
 *  completion of the generation of the random numbers:
 *  \snippet RandomArray/randomarray.c random array wait
 *  Then, we can allocate memory for the output numbers and fetch them from the
 *  device:
 *  \snippet RandomArray/randomarray.c random array read
 *  The complete code for this example can be found in RandomArray/randomarray.c.
 */
clrngStatus clrngDeviceRandomU01Array(size_t streamCount, cl_mem streams,
	size_t numberCount, cl_mem outBuffer, cl_uint numQueuesAndEvents,
	cl_command_queue* commQueues, cl_uint numWaitEvents,
	const cl_event* waitEvents, cl_event* outEvents);


/*! @} */


/*! @name Miscellaneous Functions
 */
/*! @{ */

/*! @brief Format and output information about a stream object to a file
 *
 *  @param[in]  stream  Stream object about which to write information.
 *  @param[in]  file    File to which to output.  Can be set to `stdout` or
 *                      `stderr` for standard output and error.
 *  @return     Error status
 */
clrngStatus clrngWriteStreamInfo(const clrngStream* stream, FILE *file);


/*! @} */


#ifdef __cplusplus
}
#endif

/*
 * vim: syntax=c.doxygen spell spelllang=en fdm=syntax fdl=1
 */

#endif
