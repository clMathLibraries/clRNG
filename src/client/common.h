
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

#ifndef COMMON_H
#define COMMON_H 

#ifdef __APPLE__
#include <Opencl/cl.h>
#else
#include <CL/cl.h>
#endif

#include <stdio.h>

#if defined ( WIN32 )
#define __func__ __FUNCTION__
#endif

#ifndef __cplusplus
typedef enum b_ 
{ 
	false = 0, 
	true =1 
} bool;
#endif

typedef enum simType_ {
	CPU_Exec = 1,
	GPU_Exec = 2
} simType;

typedef struct simResult_ {
	int ExecOption; // menu option executed = 1..7
	simType SimType;     // CPU or GPU
	//Results
	float CPU_time;
	float average;
	float variance;
	float CI_low;
	float CI_high;

} simResult;

typedef struct policyPoint_{
	int s, S;
} policyPoint;

//Confidence Interval calculation
void computeCI(int n, double* statTally, simResult * results);

/*! @brief Interrupt the program if an error has occurred.
 *
 *  Print the error message \c msg to standard error and exists the program if
 *  \c errcode < 0.
 *  If \c msg is NULL, clrngGetErrorString() is invoked to obtain the message
 *  string.
 */
void check_error(cl_int errcode, const char* msg, ...);

/*! @brief Reads a whole file in a buffer.
 *
 *  Necessary memory is allocated and must be released manually with free().
 */
int read_file(const char* filename, char** pbuf);

/*! @brief Retrieve the specified OpenCL device name.
 *
 *  A pointer to a static memory location is returned.
 *  It is overwritten at each call go get_device_name().
 */
const char* get_device_name(cl_device_id device);

/*! @brief Retrieve the specified OpenCL device version.
 *
 *  A pointer to a static memory location is returned.
 *  It is overwritten at each call go get_device_version().
 */
const char* get_device_version(cl_device_id device);

/*! @brief Retrieve the specified OpenCL platform name.
 *
 *  A pointer to a static memory location is returned.
 *  It is overwritten at each call go get_platform_name().
 */
const char* get_platform_name(cl_platform_id platform);

/*! @brief Retrieve the specified OpenCL platform version.
 *
 *  A pointer to a static memory location is returned.
 *  It is overwritten at each call go get_platform_version().
 */
const char* get_platform_version(cl_platform_id platform);

/*! @brief Return the maximum workgroup size on the given device.
 *
 *  @note The program displays an error message and is interrupted upon error.
 */
size_t get_max_workgroup_size(cl_device_id device);

/*! @brief Write the build log to \c file.
 *
 *  @note The program displays an error message and is interrupted upon error.
 */
void write_build_log(FILE* file, cl_program program, cl_device_id device);

/*! @brief Type of path for use with load_kernel_from_file()
 *
 */
typedef enum PathType_ { PATH_ABSOLUTE, PATH_RELATIVE_TO_LIB } PathType;

/*! @brief Create and build an OpenCL probram from a source file.
 *  @param[in] context		OpenCL context.
 *  @param[in] device		OpenCL device ID.
 *  @param[in] source_file	Path to the source file, relative to the
 *				library root specified by the environment
 *				variable CLRNG_ROOT if \c
 *				relative_to_lib_root is
 *				\c PATH_RELATIVE_TO_LIB.
 *  @param[in] path_type	If PATH_RELATIVE_TO_LIB, \c source_file is
 *				considered to be a path relative to the
 *				library root.
 *  @param[in] extra_options    Additional options to pass to the OpenCL C
 *				compiler.
 *  @return Created and built OpenCL kernel.
 *
 *  @note The program displays an error message and is interrupted upon error.
 */
cl_program build_program_from_file(
	cl_context context,
	cl_device_id device,
	const char* source_file,
	PathType path_type,
	const char* extra_options);

/*! Prepare the OpenCL environment and run a given task.
 *
 *  The task is specified as a callback function.
 *  The OpenCL resources for the context, device and command queue are managed
 *  by this function.  The task callback is responsible for managing its
 *  buffers and kernels.
 *  The context, device and command queue that are passed to the task function
 *  must not be released by the user; they are managed by call_with_opencl().
 *
 *  @param[in] platform_index   The OpenCL platform with corresponding index is selected.
 *  @param[in] task		Callback function.
 *  @param[in] device_type	CL_DEVICE_TYPE_CPU or CL_DEVICE_TYPE_GPU.
 *  @param[in] device_index     If < 0, the task is run for all devices, otherwise, the
 *				device with corresponding index is selected.
 *  @param[in] data		Extra data to pass as the last argument to the
 *				callback (can be NULL).
 *  @param[in] echoVersion		Used to activate the display of information about Platform/Device versions.
 */
int call_with_opencl(
	int platform_index,
	cl_device_type device_type,
	int device_index,
	int (*task)(cl_context,cl_device_id,cl_command_queue,void*),
	void* data,
	bool echoVersion);
#endif
