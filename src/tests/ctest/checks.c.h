
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

/*! @file checks.c.h
 *  @brief Tests that do not depend on the floating-point precision.
 *
 *  These tests must be compiled once for every generator.
 */

#include "mangle.h"
#include "util.h"
#include "../client/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS_STR(ret) ((ret) == EXIT_SUCCESS ? "  SUCCESS" : "* FAILURE")

// defined in *_common.c
cl_long CTEST_MANGLE(compareState)(const clrngStreamState* state1, const clrngStreamState* state2);
void CTEST_MANGLE(writeState)(FILE* file, const clrngStreamState* state);
// for MSVC
cl_long ctestMrg31k3p_compareState(const clrngStreamState* state1, const clrngStreamState* state2);
void    ctestMrg31k3p_writeState  (FILE* file, const clrngStreamState* state);
cl_long ctestMrg32k3a_compareState(const clrngStreamState* state1, const clrngStreamState* state2);
void    ctestMrg32k3a_writeState  (FILE* file, const clrngStreamState* state);
cl_long ctestLfsr113_compareState(const clrngStreamState* state1, const clrngStreamState* state2);
void    ctestLfsr113_writeState  (FILE* file, const clrngStreamState* state);
cl_long ctestPhilox432_compareState(const clrngStreamState* state1, const clrngStreamState* state2);
void    ctestPhilox432_writeState  (FILE* file, const clrngStreamState* state);


/*! @brief Check successive states.
 *  
 *  Requires `CTEST_MANGLE(expectedSuccessiveStates)[]` to be defined.
 */
int CTEST_MANGLE(checkSuccessiveStates)()
{
  int ret = EXIT_SUCCESS;
  size_t num_states = CTEST_ARRAY_SIZE(CTEST_MANGLE(expectedSuccessiveStates));
  clrngStatus err;

  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);

  clrngStream* stream = clrngCreateStreams(creator, 1, NULL, &err);
  check_error(err, NULL);

  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);

  for (size_t i = 0; i < num_states && ret == EXIT_SUCCESS; i++) {
    if (ctestVerbose >= 2)
      CTEST_MANGLE(writeState)(stdout, &stream->current);
    if (CTEST_MANGLE(compareState)(&stream->current, &CTEST_MANGLE(expectedSuccessiveStates)[i])) {
      ret = EXIT_FAILURE;
      if (ctestVerbose) {
        printf("\n%4sStates do not match at iteration %" SIZE_T_FORMAT ".\n", "", i);
        printf("%4sWith clrngRandomU01() states:\n", "");
        printf("%8s", "");
        CTEST_MANGLE(writeState)(stdout, &stream->current);
        printf("%4sExpected state:\n", "");
        printf("%8s", "");
        CTEST_MANGLE(writeState)(stdout, &CTEST_MANGLE(expectedSuccessiveStates)[i]);
        printf("\n");
      }
    }
    clrngRandomU01(stream);
  }

  err = clrngDestroyStreams(stream);
  check_error(err, NULL);

  printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s successive states\n",
      SUCCESS_STR(ret), num_states, RNG_TYPE_S);

  return ret;
}


/*! @brief Check clrngRewindStreams()
 */
int CTEST_MANGLE(checkRewindStreams)()
{
  int ret = EXIT_SUCCESS;
  size_t num_repeat = 5;
  size_t num_streams = 10;
  size_t num_steps = 30;
  clrngStatus err;

  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);

  clrngStream* streams1 = clrngCreateStreams(creator, num_streams, NULL, &err);
  check_error(err, NULL);

  err = clrngRewindStreamCreator(creator);
  check_error(err, NULL);

  clrngStream* streams2 = clrngCreateStreams(creator, num_streams, NULL, &err);
  check_error(err, NULL);

  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);

  for (size_t i = 0; i < num_repeat && ret == EXIT_SUCCESS; i++) {

    // advance streams by generating numbers
    for (size_t j = 0; j < num_streams; j++)
      for (size_t k = 0; k < num_steps; k++)
        clrngRandomU01(&streams2[j]);

    err = clrngRewindStreams(num_streams, streams2);
    check_error(err, NULL);

    for (size_t j = 0; j < num_streams && ret == EXIT_SUCCESS; j++) {
      if (CTEST_MANGLE(compareState)(&streams1[j].current, &streams2[j].current)) {
        ret = EXIT_FAILURE;
        if (ctestVerbose) {
          printf("\n%4sStates at index %" SIZE_T_FORMAT " do not match at iteration %" SIZE_T_FORMAT ".\n", "", j, i);
          printf("%4soriginal state:\n", "");
          printf("%8s", "");
          CTEST_MANGLE(writeState)(stdout, &streams1[j].current);
          printf("%4sExpected state:\n", "");
          printf("%8s", "");
          CTEST_MANGLE(writeState)(stdout, &streams2[j].current);
          printf("\n");
        }
      }
    }
  }

  err = clrngDestroyStreams(streams1);
  check_error(err, NULL);

  err = clrngDestroyStreams(streams2);
  check_error(err, NULL);

  printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s clrngRewindStreams()\n",
      SUCCESS_STR(ret), num_repeat * num_streams, RNG_TYPE_S);

  return ret;
}


/*! @brief Check states of successively created streams with default creator.
 *  
 *  Requires `CTEST_MANGLE(expectedCreateStreamsStates)[]` to be defined.
 */
int CTEST_MANGLE(checkCreateStreams)()
{
  int ret = EXIT_SUCCESS;
  size_t num_streams = CTEST_ARRAY_SIZE(CTEST_MANGLE(expectedCreateStreamsStates));
  clrngStatus err;
  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);
  for (size_t i = 0; i < num_streams && ret == EXIT_SUCCESS; i++) {
    clrngStream* stream = clrngCreateStreams(creator, 1, NULL, &err);
    check_error(err, NULL);
    if (ctestVerbose >= 2)
      CTEST_MANGLE(writeState)(stdout, &stream->current);
    if (CTEST_MANGLE(compareState)(&stream->current, &CTEST_MANGLE(expectedCreateStreamsStates)[i])) {
      ret = EXIT_FAILURE;
      if (ctestVerbose) {
        printf("\n%4sStates do not match at iteration %" SIZE_T_FORMAT ".\n", "", i);
        printf("%4sWith clrngCreateStreams():\n", "");
        printf("%8s", "");
        CTEST_MANGLE(writeState)(stdout, &stream->current);
        printf("%4sExpected state:\n", "");
        printf("%8s", "");
        CTEST_MANGLE(writeState)(stdout, &CTEST_MANGLE(expectedCreateStreamsStates)[i]);
        printf("\n");
      }
    }
    err = clrngDestroyStreams(stream);
    check_error(err, NULL);
  }
  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);

  printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s clrngCreateStreams()\n",
      SUCCESS_STR(ret), num_streams, RNG_TYPE_S);

  return ret;
}


/*! @brief Checks that clrngRewindStreamCreator() works as expected.
 *
 * Checks that a stream creator recreate the same streams after a call to clrngRewindStreamCreator().
 */
int CTEST_MANGLE(checkRewindStreamCreator)()
{
  int ret = EXIT_SUCCESS;
  size_t num_repeat = 5;
  size_t num_streams = 10;
  clrngStatus err;
  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);
  clrngStream* streams1 = clrngCreateStreams(creator, num_streams, NULL, &err);
  check_error(err, NULL);
  for (size_t i = 0; i < num_repeat && ret == EXIT_SUCCESS; i++) {
    err = clrngRewindStreamCreator(creator);
    check_error(err, NULL);
    clrngStream* streams2 = clrngCreateStreams(creator, num_streams, NULL, &err);
    check_error(err, NULL);
    for (size_t j = 0; j < num_streams && ret == EXIT_SUCCESS; j++) {
      if (CTEST_MANGLE(compareState)(&streams1[j].current,   &streams2[j].current) ||
          CTEST_MANGLE(compareState)(&streams1[j].initial,   &streams2[j].initial) ||
          CTEST_MANGLE(compareState)(&streams1[j].substream, &streams2[j].substream)) {
        ret = EXIT_FAILURE;
        if (ctestVerbose) {
          printf("\n%4sStreams at index %" SIZE_T_FORMAT " do not match at iteration %" SIZE_T_FORMAT ".\n", "", j, i);
          printf("%4soriginal stream:\n", "");
          printf("%8s", "");
          CTEST_MANGLE(writeState)(stdout, &streams1->current);
          printf("%8s", "");
          CTEST_MANGLE(writeState)(stdout, &streams1->initial);
          printf("%8s", "");
          CTEST_MANGLE(writeState)(stdout, &streams1->substream);
          printf("%4srecreated stream:\n", "");
          printf("%8s", "");
          CTEST_MANGLE(writeState)(stdout, &streams2->current);
          printf("%8s", "");
          CTEST_MANGLE(writeState)(stdout, &streams2->initial);
          printf("%8s", "");
          CTEST_MANGLE(writeState)(stdout, &streams2->substream);
          printf("\n");
        }
      }
    }
    err = clrngDestroyStreams(streams2);
    check_error(err, NULL);
  }

  err = clrngDestroyStreams(streams1);
  check_error(err, NULL);

  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);

  printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s clrngRewindStreamCreator()\n",
      SUCCESS_STR(ret), num_repeat * num_streams, RNG_TYPE_S);

  return ret;
}


/*! @brief Check states of successively created streams using a creator with
 *  a different spacing.
 *
 *  The states are validated against clrngAdvanceStreams().
 */
int CTEST_MANGLE(checkChangeStreamsSpacingHelper)(size_t num_streams, cl_int e, cl_int c)
{
  int ret = EXIT_SUCCESS;
  clrngStatus err;
  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);
  err = clrngChangeStreamsSpacing(creator, e, c);
  check_error(err, NULL);

  // create reference stream
  clrngStreamCreator* creator2 = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);
  clrngStream* stream2 = clrngCreateStreams(creator2, 1, NULL, &err);
  check_error(err, NULL);
  err = clrngDestroyStreamCreator(creator2);
  check_error(err, NULL);

  for (size_t i = 0; i < num_streams && ret == EXIT_SUCCESS; i++) {

    clrngStream* stream = clrngCreateStreams(creator, 1, NULL, &err);
    check_error(err, NULL);

    if (ctestVerbose >= 2)
      CTEST_MANGLE(writeState)(stdout, &stream->current);

    if (CTEST_MANGLE(compareState)(&stream->current, &stream2->current)) {
      ret = EXIT_FAILURE;
      if (ctestVerbose) {
        printf("\n%4sStates do not match at iteration %" SIZE_T_FORMAT ".\n", "", i);
        printf("%4sWith clrngCreateStreams():\n", "");
        printf("%8s", "");
        CTEST_MANGLE(writeState)(stdout, &stream->current);
        printf("%4sWith clrngAdvanceStreams():\n", "");
        printf("%8s", "");
        CTEST_MANGLE(writeState)(stdout, &stream2->current);
        printf("\n");
      }
    }

    err = clrngDestroyStreams(stream);
    check_error(err, NULL);

    err = clrngAdvanceStreams(1, stream2, e, c);
    check_error(err, NULL);
  }

  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);

  err = clrngDestroyStreams(stream2);
  check_error(err, NULL);

  printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s clrngChangeStreamsSpacing(%d,%d)\n",
      SUCCESS_STR(ret), num_streams, RNG_TYPE_S, e, c);

  return ret;
}

int CTEST_MANGLE(checkChangeStreamsSpacing)()
{
  int ret = 0;
#ifdef CTEST_NO_ADVANCE
  fprintf(stderr, "  SKIP     %8s        -  %10s clrngChangeStreamsSpacing()\n", "", RNG_TYPE_S);
#else
  size_t repeat = 10;
  struct e_c_type {
    cl_int e;
    cl_int c;
  };
#ifdef CTEST_STREAMS_SPACING_MULTIPLE_OF_4
  struct e_c_type e_c[] = { {0,4},        {2,0}, {0,32}, {10,0}, {10,32}, {10,-32} };
#else
  struct e_c_type e_c[] = { {0,2}, {1,0}, {2,0}, {0,30}, {10,0}, {10,30}, {10,-30} };
#endif

  for (size_t i = 0; i < CTEST_ARRAY_SIZE(e_c); i++)
    ret |= CTEST_MANGLE(checkChangeStreamsSpacingHelper)(repeat, e_c[i].e, e_c[i].c);
#endif

  return ret;
}


/*! @brief Check skip-ahead against sucessive states.
 *  
 *  Advance `stream` `num_repeat` times by a jump of size `sign(e) 2^e + c`.
 *
 *  With verbose `>= 2`: Output the initial state plus the `num_repeat` states
 *  visited with `num_repeat` successive jumps.
 */
int CTEST_MANGLE(checkAdvanceStreamsHelper)(clrngStream* stream, cl_int e, cl_int c, size_t num_repeat)
{
  int ret = EXIT_SUCCESS;

  cl_int sign_e = e == 0 ? 0 : e < 0 ? -1 : 1;
  long jump = sign_e * (1 << (sign_e * e)) + c;

  clrngStatus err;
  clrngStream* stream2 = clrngCopyStreams(1, stream, &err);
  check_error(err, NULL);

  size_t i;
  for (i = 1; i <= num_repeat; i++) {

    // output stream state
    if (ctestVerbose >= 2)
      CTEST_MANGLE(writeState)(stdout, &stream->current);

    // skip ahead with one big step
    clrngStatus err = clrngAdvanceStreams(1, stream, e, c);
    check_error(err, NULL);

    // skip ahead with many unit steps
    for (long j = 0; j < jump; j++)
      clrngRandomU01(stream2);

    // validate state
    if (CTEST_MANGLE(compareState)(&stream->current, &stream2->current)) {
      ret = EXIT_FAILURE;
      break;
    }
  }

  if (ctestVerbose >= 2 && ret == EXIT_SUCCESS) {
    CTEST_MANGLE(writeState)(stdout, &stream->current);
    printf("\n");
  }

  if (ret != EXIT_SUCCESS) {
    if (ctestVerbose) {
      printf("\n%4sStates do not match at iteration %" SIZE_T_FORMAT ".\n", "", i);
      printf("%4sWith clrngAdvanceStreams():\n", "");
      printf("%8s", "");
      CTEST_MANGLE(writeState)(stdout, &stream->current);
      printf("%4sWith successive clrngRandomU01():\n", "");
      printf("%8s", "");
      CTEST_MANGLE(writeState)(stdout, &stream2->current);
      printf("\n");
    }
  }

  err = clrngDestroyStreams(stream2);
  check_error(err, NULL);

  printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s clrngAdvanceStreams(%d,%d)\n",
      SUCCESS_STR(ret), num_repeat, RNG_TYPE_S, e, c);

  return ret;
}


int CTEST_MANGLE(checkAdvanceStreams)()
{
  int ret = 0;
#ifdef CTEST_NO_ADVANCE
  fprintf(stderr, "  SKIP     %8s        -  %10s clrngAdvanceStreams()\n", "", RNG_TYPE_S);
#else
  size_t repeat = 10;
  struct e_c_type {
    cl_int e;
    cl_int c;
  };
  struct e_c_type e_c[] = { {0,2}, {2,0}, {0,30}, {10,0}, {10,30}, {5,-31}, {10,-30}, {-10,1030}, {-5,50} };

  clrngStatus err;
  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);
  for (size_t i = 0; i < CTEST_ARRAY_SIZE(e_c); i++) {
    clrngStream* stream = clrngCreateStreams(creator, 1, NULL, &err);
    check_error(err, NULL);
    ret |= CTEST_MANGLE(checkAdvanceStreamsHelper)(stream, e_c[i].e, e_c[i].c, repeat);
    err = clrngDestroyStreams(stream);
    check_error(err, NULL);
  }
  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);
#endif

  return ret;
}


/*! @brief Create multiple streams in sequence and advance them by many substream.
 *  
 *  Check clrngForwardToNextSubstreams() against the states defined by
 *  CTEST_MANGLE(expectedForwardToNextSubstreamsStates), and against
 *  clrngAdvanceStreams() if available.
 *
 *  With verbose off: Output the initial state of the `num_substreams`-th
 *  substream of the `num_streams`-th stream.
 *
 *  With verbose `>= 2`: Output the initial states of the `num_substreams` substreams
 *  of each of the `num_streams` created streams.
 */
int CTEST_MANGLE(checkForwardToNextSubstreams)()
{
  int ret = EXIT_SUCCESS;

  size_t num_substreams = CTEST_ARRAY_SIZE(CTEST_MANGLE(expectedForwardToNextSubstreamsStates));

  clrngStatus err;
  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);

  // create base stream
  clrngStream* stream1 = clrngCreateStreams(creator, 1, NULL, &err);
  check_error(err, NULL);

  // create comparison stream
  clrngStream* stream2 = clrngCopyStreams(1, stream1, &err);
  check_error(err, NULL);

  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);

  if (ctestVerbose >= 2)
    CTEST_MANGLE(writeState)(stdout, &stream1->current);

#ifndef CTEST_NO_ADVANCE
  if (CTEST_MANGLE(compareState)(&stream1->current, &stream2->current))
    ret = EXIT_FAILURE;
#endif

  for (size_t j = 0; j < num_substreams && ret == EXIT_SUCCESS; j++) {

    // make sure clrngForwardToNextSubstreams() uses the substream initial
    // state and not the current state
    for (size_t k = 0; k < 10; k++)
      clrngRandomU01(stream1);

    err = clrngForwardToNextSubstreams(1, stream1);
    check_error(err, NULL);

    if (ctestVerbose >= 2)
      CTEST_MANGLE(writeState)(stdout, &stream1->current);

    if (CTEST_MANGLE(compareState)(&stream1->current, &CTEST_MANGLE(expectedForwardToNextSubstreamsStates)[j]))
      ret = EXIT_FAILURE;

#ifndef CTEST_NO_ADVANCE
    err = clrngAdvanceStreams(1, stream2, CTEST_SUBSTREAM_STEP_LOG2, CTEST_SUBSTREAM_STEP_LIN);
    check_error(err, NULL);

    if (CTEST_MANGLE(compareState)(&stream1->current, &stream2->current))
      ret = EXIT_FAILURE;
#endif

    if (ret != EXIT_SUCCESS && ctestVerbose) {
      printf("\n%4sStates do not match at substream %" SIZE_T_FORMAT ".\n", "", j);
      printf("%4sWith clrngForwardToNextSubstreams():\n", "");
      printf("%8s", "");
      CTEST_MANGLE(writeState)(stdout, &stream1->current);
      printf("%4sExpected:\n", "");
      printf("%8s", "");
      CTEST_MANGLE(writeState)(stdout, &CTEST_MANGLE(expectedForwardToNextSubstreamsStates)[j]);
#ifndef CTEST_NO_ADVANCE
      printf("%4sWith clrngAdvanceStreams():\n", "");
      printf("%8s", "");
      CTEST_MANGLE(writeState)(stdout, &stream2->current);
#endif
      printf("\n");
    }
  }

  err = clrngDestroyStreams(stream1);
  check_error(err, NULL);

  err = clrngDestroyStreams(stream2);
  check_error(err, NULL);

  printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s clrngForwardToNextSubstreams()%s\n",
      SUCCESS_STR(ret), num_substreams, RNG_TYPE_S,
#ifndef CTEST_NO_ADVANCE
      " [clrngAdvanceStreams()]"
#else
      ""
#endif
      );

  return ret;
}


/*! @brief Create multiple streams in sequence and advance them by many substream.
 *  
 *  Check clrngMakeSubstreams() against clrngForwardToNextSubstreams().
 */
int CTEST_MANGLE(checkMakeSubstreams)()
{
  int ret = EXIT_SUCCESS;

  size_t num_substreams = 1 << 6;

  clrngStatus err;
  clrngStreamCreator* creator = clrngCopyStreamCreator(NULL, &err);
  check_error(err, NULL);

  // create base stream
  clrngStream* stream1 = clrngCreateStreams(creator, 1, NULL, &err);
  check_error(err, NULL);

  // create substream array
  clrngStream* stream2 = clrngCopyStreams(1, stream1, &err);
  check_error(err, NULL);
  clrngStream* substreams = clrngMakeSubstreams(stream2, num_substreams, NULL, &err);
  check_error(err, NULL);
  err = clrngDestroyStreams(stream2);
  check_error(err, NULL);

  err = clrngDestroyStreamCreator(creator);
  check_error(err, NULL);

  size_t j = 0;

  if (CTEST_MANGLE(compareState)(&stream1->current, &substreams[j].current))
    ret = EXIT_FAILURE;

  for (j = 1; j < num_substreams && ret == EXIT_SUCCESS; j++) {

    err = clrngForwardToNextSubstreams(1, stream1);
    check_error(err, NULL);

    if (CTEST_MANGLE(compareState)(&stream1->current, &substreams[j].current))
      ret = EXIT_FAILURE;
  }

  if (ret != EXIT_SUCCESS && ctestVerbose) {
    printf("\n%4sStates do not match at substream %" SIZE_T_FORMAT ".\n", "", j);
    printf("%4sWith clrngForwardToNextSubstreams():\n", "");
    printf("%8s", "");
    CTEST_MANGLE(writeState)(stdout, &stream1->current);
    printf("%4sWith clrngMakeSubstreams():\n", "");
    printf("%8s", "");
    CTEST_MANGLE(writeState)(stdout, &substreams[j].current);
    printf("\n");
  }

  err = clrngDestroyStreams(stream1);
  check_error(err, NULL);

  err = clrngDestroyStreams(substreams);
  check_error(err, NULL);

  printf("%s  %8" SIZE_T_FORMAT " tests  -  %10s clrngMakeSubstreams()\n",
      SUCCESS_STR(ret), num_substreams, RNG_TYPE_S);

  return ret;
}

