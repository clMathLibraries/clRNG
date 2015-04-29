
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


if( WIN32 )
	# We require 2.8.3 for windows because of a bug in cmake that prevented vs2010 from generating
	# executables properly with multiple periods
	cmake_minimum_required( VERSION 2.8.3 )
else( )
	cmake_minimum_required( VERSION 2.6 )
endif( )

if( CMAKE_GENERATOR MATCHES "NMake" )
	option( NMAKE_COMPILE_VERBOSE "Print compile and link strings to the console" OFF )
	if( NMAKE_COMPILE_VERBOSE )
		set( CMAKE_START_TEMP_FILE "" )
		set( CMAKE_END_TEMP_FILE "" )
		set( CMAKE_VERBOSE_MAKEFILE 1 )
	endif( )
endif( )

# This becomes the name of the solution file
project( clRNG )

# Define a version for the code
if( NOT DEFINED CLRNG_VERSION_MAJOR )
  set( CLRNG_VERSION_MAJOR 1 )
endif( )

if( NOT DEFINED CLRNG_VERSION_MINOR )
  set( CLRNG_VERSION_MINOR 0 )
endif( )

if( NOT DEFINED CLRNG_VERSION_PATCH )
  set( CLRNG_VERSION_PATCH 0 )
endif( )

set( CLRNG_VERSION "${CLRNG_VERSION_MAJOR}.${CLRNG_VERSION_MINOR}.${CLRNG_VERSION_PATCH}")
	
# This is incremented when the ABI to the library changes
set( CLRNG_SOVERSION 1 )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR} )

# On windows, it's convenient to change the default install prefix such that it does NOT point to 'program files'
# Need to check out CMAKE_RUNTIME_OUTPUT_DIRECTORY variable, and see if that eliminates the need to modify install path
if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
	set( CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/package" CACHE PATH "Install path prefix, prepended onto install directories" FORCE )
endif( )

# Set the default of CMAKE_BUILD_TYPE to be release, unless user specifies with -D.  MSVC_IDE does not use CMAKE_BUILD_TYPE
if( NOT MSVC_IDE AND NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE )
endif()

# Options that the user or driver program can set to control various components of the build
option( BUILD_RUNTIME "Build the RNG runtime library" ON )
option( BUILD_CLIENT "Build a command line clRNG client program with a variety of configurable parameters" ON )
option( BUILD_TEST "Build library tests" ON )
#option( BUILD_LOADLIBRARIES "Build the optional dynamic load libraries that the RNG runtime will search for" ON )
option( BUILD_SHARED_LIBRARY "Build shared libraries." ON)
SET (LIB_TYPE STATIC)
# If BOOST_ROOT is defined as an environment value, use that value and cache it so it's visible in the cmake-gui.  
# Otherwise, create a sensible default that the user can change
#if( DEFINED ENV{BOOST_ROOT} )
#	set( BOOST_ROOT $ENV{BOOST_ROOT} CACHE PATH "Environment variable defining the root of the Boost installation" )
#endif( )

# Currently, linux has a problem outputing both narrow and wide characters,
# which happens in our client because openCL only supports narrow characters
if( WIN32 )
	option( UNICODE "Build with Unicode Support" ON )
	if( UNICODE )
		message( STATUS "UNICODE build" )
	endif( )
else()
	set( UNICODE OFF )
	message( STATUS "UNICODE feature disabled on linux" )
endif()

if( MSVC_IDE )
    set_property( GLOBAL PROPERTY USE_FOLDERS TRUE )

	set( BUILD64 ${CMAKE_CL_64} )
else()
	option( BUILD64 "Build a 64-bit product" ON )

	if( IS_DIRECTORY ${PROJECT_SOURCE_DIR}/tests )
		option( CODE_COVERAGE "Build makefiles with code coverage instrumentation" OFF )
		if( CODE_COVERAGE )
			message( STATUS "Code coverage instrumentation on" )
		endif()
	endif()
endif()

# These variables are meant to contain string which should be appended to the installation paths 
# of library and executable binaries, respectively.  They are meant to be user configurable/overridable.  
set( SUFFIX_LIB_DEFAULT "" )
set( SUFFIX_BIN_DEFAULT "" )

# Modify the global find property to help us find libraries like Boost in the correct paths for 64-bit
# Essentially, find_library calls will look for /lib64 instead of /lib; works for windows and linux
if( BUILD64 )
	set_property( GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE )
	message( STATUS "64bit build - FIND_LIBRARY_USE_LIB64_PATHS TRUE" )

    set( SUFFIX_LIB_DEFAULT "64" )
else( )
	set_property( GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS FALSE )
	message( STATUS "32bit build - FIND_LIBRARY_USE_LIB64_PATHS FALSE" )
endif( )

set( SUFFIX_LIB ${SUFFIX_LIB_DEFAULT} CACHE STRING "String to append to 'lib' install path" )
set( SUFFIX_BIN ${SUFFIX_BIN_DEFAULT} CACHE STRING "String to append to 'bin' install path" )

# Useful variables to configure FindBoost.cake
# set( Boost_USE_MULTITHREADED ON )
# set( Boost_DETAILED_FAILURE_MSG   ON )
# set( Boost_DEBUG ON )
# set( Boost_NO_SYSTEM_PATHS ON )

# Client is built only if boost is found; on windows, we need vs10 or higher
# Find Boost on the system, and configure the type of boost build we want
#if( NOT DEFINED Boost_USE_STATIC_LIBS )
#   set( Boost_USE_STATIC_LIBS   ON )
#endif( )
#
#if( NOT DEFINED Boost_USE_STATIC_RUNTIME )
#   set( Boost_USE_STATIC_RUNTIME OFF )
#endif( )
#
## This will define Boost_FOUND
#find_package( Boost 1.33.0 COMPONENTS program_options )
#if( Boost_FOUND )
#   message( STATUS "Boost_PROGRAM_OPTIONS_LIBRARY: ${Boost_PROGRAM_OPTIONS_LIBRARY}" )
#else( )
#   message( WARNING "Try setting Boost_DEBUG and Boost_DETAILED_FAILURE_MSG for more information" )
#endif( )

# This will define OPENCL_FOUND
find_package( OpenCL )

if( (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS 2.8) ) 
	message( STATUS "Cmake version 2.8 or greater needed to use GTest" )
else()
	# This will define GTEST_FOUND
	find_package( GTest )

	# Hack to get googletest v1.6 to work with vs2012
	if( MSVC11 )
		add_definitions( "/D_VARIADIC_MAX=10" )
	endif( )
endif()

# Enable building of the clACML client if both requested and all dependencies are found
if( BUILD_CLIENT )
  set( RNG_CLIENT ON )
else( )
  set( RNG_CLIENT OFF )
endif( )

# Enable building of the googletest unit test framework if requested and all dependencies are found
#if( BUILD_TEST AND GTEST_FOUND AND Boost_FOUND )
#	set( UNIT_TEST ON )
#else( )
#	message( "GoogleTest unit testing will NOT be built" )
#	set( UNIT_TEST OFF )
#endif( )

# FFLAGS depend on the compiler, grab the compiler name from the path
get_filename_component( C_COMPILER_NAME ${CMAKE_C_COMPILER} NAME_WE )
# message( "C_COMPILER_NAME: " ${C_COMPILER_NAME} )
# message( "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER} )

# Set common compile and link options
if( MSVC )
	# Following options for nMake
	message( STATUS "Detected MSVS Ver: " ${MSVC_VERSION} )

	# CMake sets huge stack frames for windows, for whatever reason.  We go with compiler default.
	string( REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" )
	string( REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" )
	string( REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}" ) 

elseif( CMAKE_COMPILER_IS_GNUCXX )
	message( STATUS "Detected GNU fortran compiler." )
	EXEC_PROGRAM( ${CMAKE_CXX_COMPILER} ARGS --version OUTPUT_VARIABLE vnum )
	STRING(REGEX REPLACE ".*([0-9])\\.([0-9])\\.([0-9]).*" "\\1\\2\\3" vnum ${vnum})
	if( ${vnum} STREQUAL "452" )
		# we only want c++0x if we're using gcc 4.5.2
		set( CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}" )
	endif()
	
	set( CMAKE_CXX_FLAGS "-pthread ${CMAKE_CXX_FLAGS}" )
	set( CMAKE_C_FLAGS "-std=c11 -Wall -pedantic-errors -pthread ${CMAKE_C_FLAGS}" )
	
	if( BUILD64 )
		set( CMAKE_CXX_FLAGS "-m64 ${CMAKE_CXX_FLAGS}" )
		set( CMAKE_C_FLAGS "-m64 ${CMAKE_C_FLAGS}" )
	else( )
		set( CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}" )
		set( CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}" )
	endif( )

    if( CODE_COVERAGE )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    endif()
endif( )

# If UNICODE is defined for microsoft compilers, pass extra definitions
if( MSVC AND UNICODE )
	add_definitions( "/DUNICODE /D_UNICODE" )
endif( )

# Print out compiler flags for viewing/debug
message( STATUS "CMAKE_CXX_COMPILER flags: " ${CMAKE_CXX_FLAGS} )
message( STATUS "CMAKE_CXX_COMPILER debug flags: " ${CMAKE_CXX_FLAGS_DEBUG} )
message( STATUS "CMAKE_CXX_COMPILER release flags: " ${CMAKE_CXX_FLAGS_RELEASE} )
message( STATUS "CMAKE_CXX_COMPILER relwithdebinfo flags: " ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} )
message( STATUS "CMAKE_EXE_LINKER link flags: " ${CMAKE_EXE_LINKER_FLAGS} )

# configure a header file to pass the CMake version settings to the source, and package the header files in the output archive
configure_file( "${PROJECT_SOURCE_DIR}/include/clRNG.version.h.in" "${PROJECT_BINARY_DIR}/include/clRNG.version.h" )
install( FILES 
  "${PROJECT_BINARY_DIR}/include/clRNG.version.h" 
  "include/clRNG.h"
  "include/mrg31k3p.h"
  "include/mrg32k3a.h"
  "include/lfsr113.h"
  "include/philox432.h"
  DESTINATION 
  "./include" )

install( FILES 
  "cl/include/clRNG.clh"
  "cl/include/mrg31k3p.clh"
  "cl/include/mrg32k3a.clh"
  "cl/include/lfsr113.clh"
  "cl/include/philox432.clh"
  DESTINATION 
  "./cl/include" )

install( FILES 
  "cl/include/private/modular.c.h"
  "cl/include/private/mrg31k3p.c.h"
  "cl/include/private/mrg32k3a.c.h"
  "cl/include/private/lfsr113.c.h"
  "cl/include/private/philox432.c.h"
  DESTINATION 
  "./cl/include/private" )

install( DIRECTORY
  "cl/include/private/Random123"
  DESTINATION 
  "./cl/include/private" )

# Recurse into subdirectory and start building files there
if( BUILD_RUNTIME AND IS_DIRECTORY "${PROJECT_SOURCE_DIR}/library" )
	add_subdirectory( library )
else()
	message( "Runtime library will NOT be built" )
endif( )

if( IS_DIRECTORY "${PROJECT_SOURCE_DIR}/scripts/perf" )
	add_subdirectory( scripts/perf )
endif( )

# We only want to build the following if the user options are set
if( RNG_CLIENT AND IS_DIRECTORY "${PROJECT_SOURCE_DIR}/client" )
	add_subdirectory( client )
else( )
  message( "RNG clients will NOT be built" )
endif( )

# Recurse into subdirectory and start building files there
#if( BUILD_LOADLIBRARIES AND IS_DIRECTORY "${PROJECT_SOURCE_DIR}/statTimer" )
#	add_subdirectory( statTimer )
#else()
#	message( "LoadLibraries will NOT be built" )
#endif( )

if( BUILD_TEST AND IS_DIRECTORY "${PROJECT_SOURCE_DIR}/tests" )
	# enable_testing( )
	add_subdirectory( tests )
else( )
	message( "Library tests will NOT be built" )
endif( )

# The following code is setting variables to control the behavior of CPack to generate our 
if( WIN32 )
	set( CPACK_SOURCE_GENERATOR "ZIP" )
	set( CPACK_GENERATOR "ZIP" )
else( )
	set( CPACK_SOURCE_GENERATOR "TGZ" )
	set( CPACK_GENERATOR "TGZ" )
endif( )

if( BUILD64 )
  set( CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CLRNG_VERSION}-${CMAKE_HOST_SYSTEM_NAME}-x64")
else( )
  set( CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CLRNG_VERSION}-${CMAKE_HOST_SYSTEM_NAME}-x32")
endif( )

set( CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CLRNG_VERSION}-${CMAKE_HOST_SYSTEM_NAME}-Source")

set( CPACK_PACKAGE_VERSION_MAJOR ${CLRNG_VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${CLRNG_VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${CLRNG_VERSION_PATCH} )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenCL implementation of an RNG Streams library")
set( CPACK_PACKAGE_VENDOR "Neutral")
set( CPACK_SOURCE_IGNORE_FILES "/\\\\.hg/;/\\\\.svn/;/\\\\.git/" )

# Define all variables that influence CPack before including CPack, such as install targets
include( CPack )
