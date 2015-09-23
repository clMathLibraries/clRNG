
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



set( Common.Headers common.h ../include/clRNG/clRNG.h ../include/clRNG/mrg31k3p.h )
set( Common.Source  common.c )

#  workitem
set( WorkItem.Source WorkItem/workitem.c ${Common.Source} )
set( WorkItem.Headers ${Common.Headers} )
set( WorkItem.Files ${WorkItem.Source} ${WorkItem.Headers} WorkItem/workitem_kernel.cl )

#  multistream
set( MultiStream.Source MultiStream/multistream.c ${Common.Source} )
set( MultiStream.Headers ${Common.Headers} )
set( MultiStream.Files ${MultiStream.Source} ${MultiStream.Headers} )

#  hostonly
set( HostOnly.Source HostOnly/hostonly.c ${Common.Source} )
set( HostOnly.Headers ${Common.Headers} )
set( HostOnly.Files ${HostOnly.Source} ${HostOnly.Headers} )

#  randomarray
set( RandomArray.Source RandomArray/randomarray.c ${Common.Source} )
set( RandomArray.Headers ${Common.Headers} )
set( RandomArray.Files ${RandomArray.Source} ${RandomArray.Headers} )

set( Inventory.Source
        Inventory/inventory.c
        Inventory/Policies.c
        Inventory/SimulateRuns.c 
        ${Common.Source} )
set( Inventory.Headers
        Inventory/Policies.h
        Inventory/SimulateRuns.h
        ${Common.Headers} )
set( Inventory.Files ${Inventory.Source} ${Inventory.Headers}
        Inventory/InventoryKernels.cl )

set( Client.Source ${WorkItem.Source} ${MultiStream.Source} ${HostOnly.Source} ${RandomArray.Source} ${Inventory.Source} )

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

# Include standard OpenCL headers
include_directories( ${OPENCL_INCLUDE_DIRS} ${PROJECT_BINARY_DIR}/include ../include )

add_executable( WorkItem    ${WorkItem.Files} )
add_executable( MultiStream ${MultiStream.Files} )
add_executable( HostOnly    ${HostOnly.Files} )
add_executable( RandomArray ${RandomArray.Files} )
add_executable( Inventory   ${Inventory.Files} )

target_link_libraries( WorkItem     clRNG ${OPENCL_LIBRARIES} ${DL_LIB} ${MATH_LIB} )
target_link_libraries( MultiStream  clRNG ${OPENCL_LIBRARIES} ${DL_LIB} ${MATH_LIB} )
target_link_libraries( HostOnly     clRNG ${OPENCL_LIBRARIES} ${DL_LIB} ${MATH_LIB} )
target_link_libraries( RandomArray  clRNG ${OPENCL_LIBRARIES} ${DL_LIB} ${MATH_LIB} )
target_link_libraries( Inventory    clRNG ${OPENCL_LIBRARIES} ${DL_LIB} ${MATH_LIB} )

set_target_properties( WorkItem     PROPERTIES VERSION ${CLRNG_VERSION} )
set_target_properties( WorkItem     PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )

set_target_properties( MultiStream  PROPERTIES VERSION ${CLRNG_VERSION} )
set_target_properties( MultiStream  PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )

set_target_properties( HostOnly     PROPERTIES VERSION ${CLRNG_VERSION} )
set_target_properties( HostOnly     PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )

set_target_properties( RandomArray  PROPERTIES VERSION ${CLRNG_VERSION} )
set_target_properties( RandomArray  PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )

set_target_properties( Inventory    PROPERTIES VERSION ${CLRNG_VERSION} )
set_target_properties( Inventory    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )

# CPack configuration; include the executable into the package
install( TARGETS WorkItem MultiStream HostOnly RandomArray Inventory
        RUNTIME DESTINATION bin${SUFFIX_BIN}
        LIBRARY DESTINATION lib${SUFFIX_LIB}
        ARCHIVE DESTINATION lib${SUFFIX_LIB}/import
        )

install( FILES 
  "WorkItem/workitem_kernel.cl"
  DESTINATION 
  "./client/WorkItem" )

install( FILES 
  "Inventory/InventoryKernels.cl"
  DESTINATION 
  "./client/Inventory" )
