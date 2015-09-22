# Locate clRNG.
# Currently supports AMD APP SDK (http://developer.amd.com/sdks/AMDAPPSDK/Pages/default.aspx/)
#
# Defines the following variables:
#
#   CLRNG_FOUND - Found the clRNG library
#   CLRNG_INCLUDE_DIRS - Include directories
#
# Also defines the library variables below as normal
# variables.  These contain debug/optimized keywords when
# a debugging library is found.
#
#   CLRNG_LIBRARIES - libclRNG
#
# Accepts the following variables as input:
#
#   CLRNG_ROOT - (as a CMake or environment variable)
#                The root directory of the clRNG library
#
#-----------------------
# Example Usage:
#
#    find_package(CLRNG REQUIRED)
#    include_directories(${CLRNG_INCLUDE_DIRS})
#
#    add_executable(foo foo.cc)
#    target_link_libraries(foo ${CLRNG_LIBRARIES})
#
#-----------------------

find_path(CLRNG_INCLUDE_DIRS
    NAMES clRNG/clRNG.h clRNG/mrg31k3p.h
    HINTS
        ${CLRNG_ROOT}/include
        $ENV{CLRNG_ROOT}/include
    PATHS
        /usr/include
        /usr/local/include
    DOC "clRNG header file path"
)
mark_as_advanced( CLRNG_INCLUDE_DIRS )

# Search for 64bit libs if FIND_LIBRARY_USE_LIB64_PATHS is set to true in the global environment, 32bit libs else
get_property( LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS )

if (LIB64)
    find_library( CLRNG_LIBRARIES
        NAMES clRNG
        HINTS
            ${CLRNG_ROOT}/lib64
            $ENV{CLRNG_ROOT}/lib64
        PATHS
            /usr/lib
            /usr/lib64
        DOC "clRNG dynamic library path"
    )
else ()
    find_library( CLRNG_LIBRARIES
        NAMES clRNG
        HINTS
            ${CLRNG_ROOT}/lib
            $ENV{CLRNG_ROOT}/lib
        PATHS
            /usr/lib
        DOC "clRNG dynamic library path"
    )
endif ()
mark_as_advanced( CLRNG_LIBRARIES )

include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( CLRNG DEFAULT_MSG CLRNG_LIBRARIES CLRNG_INCLUDE_DIRS )

if( NOT CLRNG_FOUND )
    message( STATUS "FindCLRNG looked for libraries named: clRNG" )
endif()
