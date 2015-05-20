#include "boxblur.h"

#include <iostream>

BoxBlur::BoxBlur()
{
}

IntensityMap BoxBlur::calculate(IntensityMap input, int radius, bool tileable) {
    IntensityMap result = IntensityMap(input.getWidth(), input.getHeight());

    int kernelPixelAmount = (2 * radius + 1) * (2 * radius + 1);

    #pragma omp parallel for  // OpenMP
    for(int y = 0; y < input.getHeight(); y++) {
        for(int x = 0; x < input.getWidth(); x++) {
            float sum = 0.0;

            //blur kernel loops
            for(int i = -radius; i < radius; i++) {
                for(int k = -radius; k < radius; k++) {
                    int posY = handleEdges(y + i, input.getHeight(), tileable);
                    int posX = handleEdges(x + k, input.getWidth(), tileable);

                    sum += input.at(posX, posY);
                }
            }

            //normalize sum
            sum /= kernelPixelAmount;

            result.setValue(x, y, sum);
        }
    }

    return result;
}

int BoxBlur::handleEdges(int iterator, int max, bool tileable) {
    if(iterator < 0) {
        if(tileable) {
            int corrected = max + iterator;
            //failsafe, e.g. if filter size is larger than image size
            return handleEdges(corrected, max, false);
        }
        else {
            return 0;
        }
    }

    if(iterator >= max) {
        if(tileable) {
            int corrected = iterator - max;
            //failsafe, e.g. if filter size is larger than image size
            return handleEdges(corrected, max, false);
        }
        else {
            return max - 1;
        }
    }

    return iterator;
}
