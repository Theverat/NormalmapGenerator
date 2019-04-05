/********************************************************************************
 *   Copyright (C) 2015 by Simon Wendsche                                       *
 *                                                                              *
 *   This file is part of NormalmapGenerator.                                   *
 *                                                                              *
 *   NormalmapGenerator is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by       *
 *   the Free Software Foundation; either version 3 of the License, or          *
 *   (at your option) any later version.                                        *
 *                                                                              *
 *   NormalmapGenerator is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 *   GNU General Public License for more details.                               *
 *                                                                              *
 *   You should have received a copy of the GNU General Public License          *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 *                                                                              *
 *   Sourcecode: https://github.com/Theverat/NormalmapGenerator                 *
 ********************************************************************************/

#include "boxblur.h"

#include <iostream>

BoxBlur::BoxBlur()
{
}

IntensityMap BoxBlur::calculate(IntensityMap input, int radius, bool tileable) {
    IntensityMap result = IntensityMap(input.getWidth(), input.getHeight());

    int kernelPixelAmount = (2 * radius + 1) * (2 * radius + 1);

    #pragma omp parallel for  // OpenMP
    for(size_t y = 0; y < input.getHeight(); y++) {
        for(size_t x = 0; x < input.getWidth(); x++) {
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
