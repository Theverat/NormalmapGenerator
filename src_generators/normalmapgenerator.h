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

    NormalmapGenerator(IntensityMap::Mode mode, double redMultiplier, double greenMultiplier,
                       double blueMultiplier, double alphaMultiplier);
    QImage calculateNormalmap(const QImage& input, Kernel kernel, double strength = 2.0, bool invert = false, 
                              bool tileable = true, bool keepLargeDetail = true,
                              int largeDetailScale = 25, double largeDetailHeight = 1.0);
    const IntensityMap& getIntensityMap() const;

private:
    IntensityMap intensity;
    bool tileable;
    double redMultiplier, greenMultiplier, blueMultiplier, alphaMultiplier;
    IntensityMap::Mode mode;

    int handleEdges(int iterator, int maxValue) const;
    int mapComponent(double value) const;
    QVector3D sobel(const double convolution_kernel[3][3], double strengthInv) const;
    QVector3D prewitt(const double convolution_kernel[3][3], double strengthInv) const;
    int blendSoftLight(int color1, int color2) const;
};

#endif // SOBEL_H
