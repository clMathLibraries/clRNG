# clRNG Library

A library for uniform random number generation in OpenCL.

*Streams* of random numbers act as virtual random number generators.
They can be created on the host computer in unlimited numbers, and then used 
either on the host or on computing devices by work items to generate random numbers.
Each stream also has equally-spaced substreams, which are occasionally useful.
The API is currently implemented for four different RNGs, namely the 
MRG31k3p, MRG32k3a, LFSR113 and Philox-4×32-10 generators.

#### Documentation

- [**HTML Documentation**  
  (generated with Doxygen)](http://clmathlibraries.github.io/clRNG/htmldocs/index.html)
- [**Tutorial Document**  
  *clRNG*: A Random Number API with Multiple Streams for OpenCL](http://clmathlibraries.github.io/clRNG/docs/clrng-api.pdf)

## Building

Before configuring and building the library, make sure an OpenCL software
development kit is available on your platform.


### Under Linux

Run:

    cmake src
    make
    make install

By default, `make install` packages the files necessary for distribution of the
library in the `package` subdirectory.


## Installing

The environment variable `CLRNG_ROOT` must be pointed to the root of the
packaged files, or more specifically, the directory that contains the `include`
and `cl` subdirectories.

Also make sure that the clRNG shared library can be found by the dynamic
linker.  The location of the shared library can depend on the platform.  If you
installed the library on a 64-bit Linux platform, it can normally be found
under the `lib64` subdirectory of the installation directory.


### Under Linux

On a 64-bit Linux platform with a Bash-compatible shell, if the current
directory is that in which you ran `cmake src`, you can update the necessary
environment variables by typing:

    export CLRNG_ROOT=$PWD/package
    export LD_LIBRARY_PATH=$CLRNG_ROOT/lib64:$LD_LIBRARY_PATH

Then, you can check that everything works correctly by executing the unit tests
with:

    $CLRNG_ROOT/bin/CTest


## Examples

Examples can be found in `src/client`.
The compiled client program examples can be found under the `bin` subdirectory
of the installation package (`$CLRNG_ROOT/bin` under Linux).
Note that the examples expect an OpenCL GPU device to be available.

## Simple example

The simple example below shows how to use clRNG to generate random numbers 
by directly using device side headers (.clh) in your OpenCL kernel.

```c
#include <stdlib.h>
#include <string.h>

#include "clRNG.h"
#include "mrg31k3p.h"

int main( void )
{
    cl_int err;
    cl_platform_id platform = 0;
    cl_device_id device = 0;
    cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
    cl_context ctx = 0;
    cl_command_queue queue = 0;
    cl_program program = 0;
    cl_kernel kernel = 0;
    cl_event event = 0;
    cl_mem bufIn, bufOut;
    float *out;
    char *clrng_root;
    char include_str[1024];
    char build_log[4096];
    size_t i = 0;
    size_t numWorkItems = 64;
    clrngMrg31k3pStream *streams = 0;
    size_t streamBufferSize = 0;
    size_t kernelLines = 0;

    /* Sample kernel that calls clRNG device-side interfaces to generate random numbers */
    const char *kernelSrc[] = {
    "    #define CLRNG_SINGLE_PRECISION                                   \n",
    "    #include <mrg31k3p.clh>                                          \n",
    "                                                                     \n",
    "    __kernel void example(__global clrngMrg31k3pHostStream *streams, \n",
    "                          __global float *out)                       \n",
    "    {                                                                \n",
    "        int gid = get_global_id(0);                                  \n",
    "                                                                     \n",
    "        clrngMrg31k3pStream workItemStream;                          \n",
    "        clrngMrg31k3pCopyOverStreamsFromGlobal(1, &workItemStream,   \n",
    "                                                     &streams[gid]); \n",
    "                                                                     \n",
    "        out[gid] = clrngMrg31k3pRandomU01(&workItemStream);          \n",
    "    }                                                                \n",
    "                                                                     \n",
    };

    /* Setup OpenCL environment. */
    err = clGetPlatformIDs( 1, &platform, NULL );
    err = clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL );

    props[1] = (cl_context_properties)platform;
    ctx = clCreateContext( props, 1, &device, NULL, NULL, &err );
    queue = clCreateCommandQueue( ctx, device, 0, &err );

    /* Make sure CLRNG_ROOT is specified to get library path */
    clrng_root = getenv("CLRNG_ROOT");
    if(clrng_root == NULL) printf("\nSpecify environment variable CLRNG_ROOT as described\n");
    strcpy(include_str, "-I ");
    strcat(include_str, clrng_root);
    strcat(include_str, "/cl/include");

    /* Create sample kernel */
    kernelLines = sizeof(kernelSrc) / sizeof(kernelSrc[0]);
    program = clCreateProgramWithSource(ctx, kernelLines, kernelSrc, NULL, &err);
    err = clBuildProgram(program, 1, &device, include_str, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("\nclBuildProgram has failed\n");
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 4096, build_log, NULL);
        printf("%s", build_log);
    }
    kernel = clCreateKernel(program, "example", &err);

    /* Create streams */
    streams = clrngMrg31k3pCreateStreams(NULL, numWorkItems, &streamBufferSize, (clrngStatus *)&err);

    /* Create buffers for the kernel */
    bufIn = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, streamBufferSize, streams, &err);
    bufOut = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, numWorkItems * sizeof(cl_float), NULL, &err);

    /* Setup the kernel */
    err = clSetKernelArg(kernel, 0, sizeof(bufIn),  &bufIn);
    err = clSetKernelArg(kernel, 1, sizeof(bufOut), &bufOut);

    /* Execute the kernel and read back results */
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &numWorkItems, NULL, 0, NULL, &event);
    err = clWaitForEvents(1, &event);
    out = (float *)malloc(numWorkItems * sizeof(out[0]));
    err = clEnqueueReadBuffer(queue, bufOut, CL_TRUE, 0, numWorkItems * sizeof(out[0]), out, 0, NULL, NULL);

    /* Release allocated resources */
    clReleaseEvent(event);
    free(out);
    clReleaseMemObject(bufIn);
    clReleaseMemObject(bufOut);

    clReleaseKernel(kernel);
    clReleaseProgram(program);

    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);

    return 0;
}
```


## Building the documentation manually

The documentation can be generated by running `make` from within the `doc`
directory.  This requires Doxygen to be installed.
