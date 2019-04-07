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

#include "gaussianblur.h"
#include <math.h>
#include <iostream>

GaussianBlur::GaussianBlur()
{
}

IntensityMap GaussianBlur::calculate(IntensityMap &input, double radius, bool tileable) {
    IntensityMap result = IntensityMap(input.getWidth(), input.getHeight());

    gaussBlur(input, result, radius, tileable);

    return result;
}

// Gaussian blur implementation adapted from http://blog.ivank.net/fastest-gaussian-blur.html
// "Algorithm 3" was used because I could not adapt "Algorithm 4" to be tileable

void GaussianBlur::gaussBlur(IntensityMap &input, IntensityMap &result, double radius, bool tileable) {
    std::vector<double> boxes = boxesForGauss(radius, 3);

    boxBlur(input, result, ((boxes.at(0) - 1) / 2), tileable);
    boxBlur(result, input, ((boxes.at(1) - 1) / 2), tileable);
    boxBlur(input, result, ((boxes.at(2) - 1) / 2), tileable);
}

std::vector<double> GaussianBlur::boxesForGauss(double sigma, int n) {
    double wIdeal = sqrt((12 * sigma * sigma / n) + 1);
    int wl = floor(wIdeal);

    if(wl % 2 == 0)
        wl--;

    const int wu = wl + 2;

    const double mIdeal = (12 * sigma * sigma - n * wl * wl - 4 * n * wl - 3 * n) / (-4 * wl - 4);
    const int m = round(mIdeal);

    std::vector<double> sizes;

    for(int i = 0; i < n; i++) {
        sizes.push_back(i < m ? wl : wu);
    }

    return sizes;
}

void GaussianBlur::boxBlur(IntensityMap &input, IntensityMap &result, double radius, bool tileable) {
    for(size_t i = 0; i < input.getWidth() * input.getHeight(); i++) {
        result.setValue(i, input.at(i));
    }

    boxBlurH(result, input, radius, tileable);
    boxBlurT(input, result, radius, tileable);
}

void GaussianBlur::boxBlurH(IntensityMap &input, IntensityMap &result, double radius, bool tileable) {
    const int width = input.getWidth();
    const int height = input.getHeight();

    #pragma omp parallel for  // OpenMP
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            double val = 0.0;

            for(int ix = j - radius; ix < j + radius + 1; ix++) {
                const int x = handleEdges(ix, width, tileable);
                val += input.at(x, i);
            }

            result.setValue(j, i, val / (radius + radius + 1));
        }
    }
}

void GaussianBlur::boxBlurT(IntensityMap &input, IntensityMap &result, double radius, bool tileable) {
    const int width = input.getWidth();
    const int height = input.getHeight();

    #pragma omp parallel for  // OpenMP
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            double val = 0.0;

            for(int iy = i - radius; iy < i + radius + 1; iy++) {
                const int y = handleEdges(iy, height, tileable);
                val += input.at(j, y);
            }

            result.setValue(j, i, val / (radius + radius + 1));
        }
    }
}

int GaussianBlur::handleEdges(int iterator, int max, bool tileable) const {
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
