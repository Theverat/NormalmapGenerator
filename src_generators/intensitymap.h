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

#ifndef INTENSITYMAP_H
#define INTENSITYMAP_H

#include <QImage>

class IntensityMap
{
public:
    enum Mode {
        AVERAGE,
        MAX
    };

    IntensityMap();
    IntensityMap(int width, int height);
    IntensityMap(const QImage &rgbImage, Mode mode, double redMultiplier = 1.0, double greenMultiplier = 1.0,
                 double blueMultiplier = 1.0, double alphaMultiplier = 0.0);
    double at(int x, int y) const;
    double at(int pos) const;
    void setValue(int x, int y, double value);
    void setValue(int pos, double value);
    size_t getWidth() const;
    size_t getHeight() const;
    void invert();
    QImage convertToQImage() const;

private:
    std::vector< std::vector<double> > map;
};

#endif // INTENSITYMAP_H
