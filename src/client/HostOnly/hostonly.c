
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

#include <stdio.h>

/*! [clRNG header] */
#include <clRNG/mrg31k3p.h>
/*! [clRNG header] */

int main() {
    /*! [create streams] */
    clrngMrg31k3pStream* streams = clrngMrg31k3pCreateStreams(NULL, 2, NULL, NULL);
    clrngMrg31k3pStream* single = clrngMrg31k3pCreateStreams(NULL, 1, NULL, NULL);
    /*! [create streams] */
    /*! [iterate streams] */
    int count = 0;
    for (int i = 0; i < 100; i++) {
        double u = clrngMrg31k3pRandomU01(&streams[i % 2]);
        int    x = clrngMrg31k3pRandomInteger(single, 1, 6);
        if (x * u < 2) count++;
    }
    /*! [iterate streams] */
    /*! [output] */
    printf("Average of indicators = %f\n", (double)count / 100.0);
    /*! [output] */
    return 0;
}
