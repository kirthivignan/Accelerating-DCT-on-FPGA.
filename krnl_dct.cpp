/****
Copyright (c) 2020, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****/
#define M_PI (3.14159265358979323846f)
#define BUFFER_SIZE 64
#define DATA_SIZE 4096 
//TRIPCOUNT identifier
const unsigned int c_len = DATA_SIZE / BUFFER_SIZE;
const unsigned int c_size = BUFFER_SIZE;


/**
 * @brief      Simple Vector Addition Kernel Implementation
 *             
 *
 * @param[in]  in1   Read-Only Input Vector1
 * @param[in]  in2   Read-Only Input Vector2
 * @param[out] out_r Output Result Vector
 * @param[in]  size  Number of elements in the vectors
 */

extern "C" {
void krnl_vadd(int *in1,
               int *in2,
               int *out_r,
               unsigned int size
) {

    int v1_buffer[BUFFER_SIZE];   // Local memory to store vector1
    float matC[BUFFER_SIZE];
    float matD[BUFFER_SIZE];
    const float a = 0.4903926402;
    const float b = 0.46193976625;
    const float c = 0.41573480615;
    const float d = 0.27778511651;
    const float e = 0.19134171618;
    const float f= 0.097545161;
    const float g = 0.35355339059;

     float matA[64]={
        g,  a,  b,  c,  g,  d,  e,  f,
        g,  c,  e, -f, -g, -a, -b, -d,
        g,  d, -e, -a, -g,  f,  b,  c,
        g,  f, -b, -d,  g,  c, -e, -a,
        g, -f, -b,  d,  g, -c, -e,  a,
        g, -d, -e,  a, -g, -f,  b, -c,
        g, -c,  e,  f, -g,  a, -b,  d,
        g, -a,  b, -c,  g, -d,  e,  f
    };

     float matAt[64]={
            g,g,g,g,g,g,g,g,
			a,c,d,f,-f,-d,-c,-a,
			b,e,-e,-b,-b,-e,e,b,
			c,-f,-a,-d,d,a,f,-c,
			g,-g,-g,g,g,-g,-g,g,
			d,-a,f,c,-c,-f,a,-d,
			e,-b,b,-e,-e,b,-b,e,
			f,-d,c,-a,a,-c,d,f
        };


    //Per iteration of this loop perform BUFFER_SIZE vector addition
    for (unsigned int i = 0; i < size; i += BUFFER_SIZE) {
       #pragma HLS LOOP_TRIPCOUNT min=c_len max=c_len
        unsigned int chunk_size = BUFFER_SIZE;
        //boundary checks
        if ((i + BUFFER_SIZE) > size)
            chunk_size = size - i;

        read1: for (unsigned int j = 0; j < chunk_size; j++) {
           #pragma HLS LOOP_TRIPCOUNT min=c_size max=c_size
            v1_buffer[j] = in1[i + j];
        }

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            float sum = 0.0;
            for (int k = 0; k < 8; k++)
                sum = sum + matA[r * 8 + k] * v1_buffer[k * 8 + c];
            matC[r * 8 + c] = sum;

        }
    }
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            float sum = 0.0;
            for (int k = 0; k < 8; k++)
                sum = sum + matC[r * 8 + k] * matAt[k * 8 + c];
            matD[r * 8 + c] = sum;

        }
    }

        //Burst reading B and calculating C and Burst writing 
        // to  Global memory
        vadd_writeC: for (unsigned int j = 0; j < chunk_size; j++) {
           #pragma HLS LOOP_TRIPCOUNT min=c_size max=c_size
            //perform vector addition
            out_r[i+j] = matD[j];
        }

    }
}
}