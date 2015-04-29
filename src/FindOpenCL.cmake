
# ***********************************************************************
# Copyright (c) 2015 Advanced Micro Devices, Inc. 
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions 
# are met:
# 
# 1. Redistributions of source code must retain the above copyright 
# notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright 
# notice, this list of conditions and the following disclaimer in the 
# documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# ***********************************************************************

# ########################################################################



# Locate an OpenCL implementation.
# Currently supports AMD APP SDK (http://developer.amd.com/sdks/AMDAPPSDK/Pages/default.aspx/)
#
# Defines the following variables:
#
#   OPENCL_FOUND - Found the OPENCL framework
#   OPENCL_INCLUDE_DIRS - Include directories
#
# Also defines the library variables below as normal
# variables.  These contain debug/optimized keywords when
# a debugging library is found.
#
#   OPENCL_LIBRARIES - libopencl
#
# Accepts the following variables as input:
#
#   OPENCL_ROOT - (as a CMake or environment variable)
#                The root directory of the OpenCL implementation found
#
#   FIND_LIBRARY_USE_LIB64_PATHS - Global property that controls whether findOpenCL should search for
#                              64bit or 32bit libs
#-----------------------
# Example Usage:
#
#    find_package(OPENCL REQUIRED)
#    include_directories(${OPENCL_INCLUDE_DIRS})
#
#    add_executable(foo foo.cc)
#    target_link_libraries(foo ${OPENCL_LIBRARIES})
#
#-----------------------

find_path(OPENCL_INCLUDE_DIRS
    NAMES OpenCL/cl.h CL/cl.h
    HINTS
        ${OPENCL_ROOT}/include
        $ENV{AMDAPPSDKROOT}/include
        $ENV{CUDA_PATH}/include
    PATHS
        /usr/include
        /usr/local/include
    DOC "OpenCL header file path"
)
mark_as_advanced( OPENCL_INCLUDE_DIRS )

# Search for 64bit libs if FIND_LIBRARY_USE_LIB64_PATHS is set to true in the global environment, 32bit libs else
get_property( LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS )

if( LIB64 )
    find_library( OPENCL_LIBRARIES
        NAMES OpenCL
        HINTS
            ${OPENCL_ROOT}/lib
            $ENV{AMDAPPSDKROOT}/lib
            $ENV{CUDA_PATH}/lib
        DOC "OpenCL dynamic library path"
        PATH_SUFFIXES x86_64 x64
        PATHS
            /usr/lib
    )
else( )
    find_library( OPENCL_LIBRARIES
        NAMES OpenCL
        HINTS
            ${OPENCL_ROOT}/lib
            $ENV{AMDAPPSDKROOT}/lib
            $ENV{CUDA_PATH}/lib
        DOC "OpenCL dynamic library path"
        PATH_SUFFIXES x86 Win32
        PATHS
            /usr/lib
    )
endif( )
mark_as_advanced( OPENCL_LIBRARIES )

include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( OPENCL DEFAULT_MSG OPENCL_LIBRARIES OPENCL_INCLUDE_DIRS )

if( NOT OPENCL_FOUND )
    message( STATUS "FindOpenCL looked for libraries named: OpenCL" )
endif()
