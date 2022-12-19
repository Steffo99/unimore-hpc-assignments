#include <cstring>
#include <math.h>
#include "sobel.h"

void sobel(uint8_t *__restrict__ out, uint8_t *__restrict__ in, const int width, const int height)
{
#pragma HLS INTERFACE m_axi port=out offset=slave bundle=bout
#pragma HLS INTERFACE m_axi port=in offset=slave bundle=bin
#pragma HLS INTERFACE s_axilite port=width bundle=bwidth
#pragma HLS INTERFACE s_axilite port=height bundle=bheight

    const int sobelFilter[3][3] = {
        {-1, 0, 1}, 
        {-2, 0, 2}, 
        {-1, 0, 1}
    };

    // Carica le prime tre righe nel buffer
    uint8_t inBuffer[3*height];
    memcpy(inBuffer, in, 3*height*sizeof(uint8_t));

    esternoY:
    for (int y = 0; y < height - 2; y++)
    {
    #pragma HLS PIPELINE

        esternoX:
        for (int x = 0; x < width - 2; x++)
        {
            int dx = 0;
            int dy = 0;

            internoY:
            for (int k = 0; k < 3; k++)
            {
            #pragma HLS UNROLL

                const int inYOffset = ((y + k) % 3) * width;

                internoX:
                for (int z = 0; z < 3; z++)
                {
                #pragma HLS UNROLL

                    const int inXOffset = x + z;

                    const int inOffset = inYOffset + inXOffset;
                    const int inElement = inBuffer[inOffset];

                    dx += sobelFilter[k][z] * inElement;
                    dy += sobelFilter[z][k] * inElement;
                }
            }

            const int outYOffset = (y + 1) * width;
            const int outXOffset = (x + 1);
            const int outOffset = outYOffset + outXOffset;

            out[outOffset] = sqrt((float)((dx * dx) + (dy * dy)));
        }

        memcpy(inBuffer, in + (y % 3) * height, height*sizeof(uint8_t));
    }
}
