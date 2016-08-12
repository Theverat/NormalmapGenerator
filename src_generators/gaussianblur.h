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

#ifndef GAUSSIANBLUR_H
#define GAUSSIANBLUR_H

#include "intensitymap.h"

class GaussianBlur
{
public:
    GaussianBlur();
    IntensityMap calculate(IntensityMap& input, double radius, bool tileable);

private:
    std::vector<double> boxesForGauss(double sigma, int n);
    void gaussBlur(IntensityMap &input, IntensityMap &result, double radius, bool tileable);
    void boxBlur(IntensityMap &input, IntensityMap &result, double radius, bool tileable);
    void boxBlurH(IntensityMap &input, IntensityMap &result, double radius, bool tileable);
    void boxBlurT(IntensityMap &input, IntensityMap &result, double radius, bool tileable);
    int handleEdges(int iterator, int max, bool tileable) const;
};

#endif // GAUSSIANBLUR_H
