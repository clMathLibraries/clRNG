
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


# List the names of common files to compile across all platforms
set( clRNG.Source   clRNG.c
	            private.c
		    mrg32k3a.c 
	            mrg31k3p.c 
		    lfsr113.c
		    philox432.c
		    )

if( MSVC )
	if( MSVC_VERSION LESS 1800 )
		# Use C++ with Microsoft compiler
		SET_SOURCE_FILES_PROPERTIES( ${clRNG.Source} PROPERTIES LANGUAGE CXX)
	endif ()
endif( )

# Windows only uses dllmain
#if( MSVC )
#  set( clRNG.Source ${clRNG.Source} dllmain.cpp )
#endif( )

set( clRNG.Headers  private.h 
		            ../include/clRNG.h
		            ../include/mrg32k3a.h 
		            ../include/mrg31k3p.h 
		            ../include/lfsr113.h
		            ../include/philox432.h
		            )

set( clRNG.Files ${clRNG.Source} ${clRNG.Headers} )

# Include standard OpenCL headers
include_directories( ${OPENCL_INCLUDE_DIRS} ${PROJECT_BINARY_DIR}/include ../include )

if(BUILD_SHARED_LIBRARY)
  add_library( clRNG SHARED ${clRNG.Files} )
else()
  add_library( clRNG STATIC ${clRNG.Files} )
endif()
target_link_libraries( clRNG ${OPENCL_LIBRARIES} )

set_target_properties( clRNG PROPERTIES VERSION ${CLRNG_VERSION} )
set_target_properties( clRNG PROPERTIES SOVERSION ${CLRNG_SOVERSION} )
set_target_properties( clRNG PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )

if( CMAKE_COMPILER_IS_GNUCC )
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/clRNG.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/clRNG.pc @ONLY )

  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/clRNG.pc
             DESTINATION lib${SUFFIX_LIB}/pkgconfig )
endif( )

# CPack configuration; include the executable into the package
install( TARGETS clRNG
        RUNTIME DESTINATION bin${SUFFIX_BIN}
        LIBRARY DESTINATION lib${SUFFIX_LIB}
        ARCHIVE DESTINATION lib${SUFFIX_LIB}/import
        )
        
# For debug builds, include the debug runtimes into the package for testing on non-developer machines
set( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP true )
set( CMAKE_INSTALL_DEBUG_LIBRARIES true )
set( CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY true )

if( WIN32 )
  set( CLRNG_RUNTIME_DESTINATION bin${SUFFIX_BIN} )
else( )
  set( CLRNG_RUNTIME_DESTINATION lib${SUFFIX_LIB} )
endif( )

include( InstallRequiredSystemLibraries )

# Install necessary runtime files for debug builds
install(    PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
            CONFIGURATIONS Debug
	    DESTINATION ${CLRNG_RUNTIME_DESTINATION} )

# Install all *.pdb files for debug builds
install(    DIRECTORY ${PROJECT_BINARY_DIR}/staging/
  DESTINATION ${CLRNG_RUNTIME_DESTINATION}
            OPTIONAL
            CONFIGURATIONS Debug
            FILES_MATCHING PATTERN "*.pdb" )

# Install a snapshot of the source as it was for this build; useful for the .pdb's
install(    DIRECTORY ${PROJECT_SOURCE_DIR}
  DESTINATION ${CLRNG_RUNTIME_DESTINATION}
            OPTIONAL
            CONFIGURATIONS Debug )
