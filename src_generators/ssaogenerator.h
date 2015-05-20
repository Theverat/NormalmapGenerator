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

#ifndef SSAOGENERATOR_H
#define SSAOGENERATOR_H

#include "intensitymap.h"
#include <QImage>

//code is based on http://john-chapman-graphics.blogspot.de/2013/01/ssao-tutorial.html
class SsaoGenerator
{
public:
    SsaoGenerator();
    QImage calculateSsaomap(QImage normalmap, QImage depthmap, float radius, unsigned int kernelSamples, unsigned int noiseSize);

private:
    std::vector<QVector3D> generateKernel(unsigned int size);
    std::vector<QVector3D> generateNoise(unsigned int size);

    double random(double min, double max);
    float lerp(float v0, float v1, float t);
};

#endif // SSAOGENERATOR_H
