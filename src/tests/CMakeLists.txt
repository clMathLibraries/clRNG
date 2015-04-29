
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

set( clRNG.CTest.Source
  ctest/cli.c
  ctest/util.c
  ctest/mrg31k3p_dispatch.c
  ctest/mrg31k3p_common.c
  ctest/mrg31k3p_checks_d.c
  ctest/mrg31k3p_checks_s.c
  ctest/mrg32k3a_dispatch.c
  ctest/mrg32k3a_common.c
  ctest/mrg32k3a_checks_d.c
  ctest/mrg32k3a_checks_s.c
  ctest/lfsr113_dispatch.c
  ctest/lfsr113_common.c
  ctest/lfsr113_checks_d.c
  ctest/lfsr113_checks_s.c
  ctest/philox432_dispatch.c
  ctest/philox432_common.c
  ctest/philox432_checks_d.c
  ctest/philox432_checks_s.c
  ${PROJECT_SOURCE_DIR}/client/common.c
)
set( clRNG.CTest.Headers
  ${PROJECT_SOURCE_DIR}/include/clRNG.h
  ${PROJECT_SOURCE_DIR}/include/mrg31k3p.h
  ${PROJECT_SOURCE_DIR}/include/mrg32k3a.h
  ${PROJECT_SOURCE_DIR}/include/lfsr113.h
  ${PROJECT_SOURCE_DIR}/include/philox432.h
  ${PROJECT_SOURCE_DIR}/client/common.h
  ctest/util.h
  ctest/mangle.h
  ctest/checks.h
  ctest/dispatch.c.h
  ctest/checks.c.h
  ctest/checks_prec.c.h
)

set( clRNG.CTest.Files ${clRNG.CTest.Source} ${clRNG.CTest.Headers} )
add_executable( CTest ${clRNG.CTest.Files} )

include_directories(
  ${OPENCL_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include
)

set( Client.Source ${clRNG.CTest.Source} )

if( MSVC )
	if( MSVC_VERSION LESS 1800 )
		# Use C++ with Microsoft compiler
		SET_SOURCE_FILES_PROPERTIES( ${Client.Source} PROPERTIES LANGUAGE CXX)
	endif ()
endif( )
		
set( DL_LIB "" )
if( WIN32 )
        add_definitions( "/D_CONSOLE" )
elseif( APPLE )
        set( CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}" )   
else( )
        # To use the dlopen() and dlclose() functions, we should link with libdl
        set( DL_LIB "-ldl" )
endif( )

if( CMAKE_COMPILER_IS_GNUCC )
    set( MATH_LIB "-lm" )
endif()

if( NOT BUILD_RUNTIME )
  message( ERROR "Building tests requires building runtime." )
endif( )

target_link_libraries( CTest clRNG ${OPENCL_LIBRARIES} ${DL_LIB} ${MATH_LIB} )

set_target_properties( CTest PROPERTIES VERSION ${CLRNG_VERSION} )
set_target_properties( CTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )

# CPack configuration; include the executable into the package
install( TARGETS CTest
        RUNTIME DESTINATION bin${SUFFIX_BIN}
        )

