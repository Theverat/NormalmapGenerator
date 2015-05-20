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

#ifndef SOBEL_H
#define SOBEL_H

#include <QImage>
#include "intensitymap.h"

class NormalmapGenerator
{
public:
    enum Kernel {
        SOBEL,
        PREWITT
    };

    NormalmapGenerator(IntensityMap::Mode mode, bool useRed, bool useGreen, bool useBlue, bool useAlpha);
    QImage calculateNormalmap(QImage input, Kernel kernel, double strength = 2.0, bool invert = false, 
                              bool tileable = true, bool keepLargeDetail = true, int largeDetailScale = 25, double largeDetailHeight = 1.0);
    IntensityMap getIntensityMap();

private:
    IntensityMap intensity;
    bool tileable;
    bool useRed, useGreen, useBlue, useAlpha;
    IntensityMap::Mode mode;

    int handleEdges(int iterator, int max);
    int mapComponent(double value);
    QVector3D sobel(double convolution_kernel[3][3], double strength);
    QVector3D prewitt(double convolution_kernel[3][3], double strength);
    int blendSoftLight(int color1, int color2);
};

#endif // SOBEL_H
