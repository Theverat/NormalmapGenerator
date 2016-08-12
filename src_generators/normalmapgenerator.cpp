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

#include "normalmapgenerator.h"
#include <QVector3D>
#include <QColor>

NormalmapGenerator::NormalmapGenerator(IntensityMap::Mode mode, bool useRed, bool useGreen, bool useBlue, bool useAlpha)
    : tileable(false), useRed(useRed), useGreen(useGreen), useBlue(useBlue), useAlpha(useAlpha), mode(mode)
{}

const IntensityMap& NormalmapGenerator::getIntensityMap() const {
    return this->intensity;
}

QImage NormalmapGenerator::calculateNormalmap(const QImage& input, Kernel kernel, double strength, bool invert, bool tileable, 
                                              bool keepLargeDetail, int largeDetailScale, double largeDetailHeight) {
    this->tileable = tileable;

    this->intensity = IntensityMap(input, mode, useRed, useGreen, useBlue, useAlpha);
    if(!invert) {
        // The default "non-inverted" normalmap looks wrong in renderers,
        // so I use inversion by default
        intensity.invert();
	}

    QImage result(input.width(), input.height(), QImage::Format_ARGB32);
    
    // optimization
    double strengthInv = 1.0 / strength;

    #pragma omp parallel for  // OpenMP
    //code from http://stackoverflow.com/a/2368794
    for(int y = 0; y < input.height(); y++) {
        QRgb *scanline = (QRgb*) result.scanLine(y);

        for(int x = 0; x < input.width(); x++) {

            const double topLeft      = intensity.at(handleEdges(x - 1, input.width()), handleEdges(y - 1, input.height()));
            const double top          = intensity.at(handleEdges(x - 1, input.width()), handleEdges(y,     input.height()));
            const double topRight     = intensity.at(handleEdges(x - 1, input.width()), handleEdges(y + 1, input.height()));
            const double right        = intensity.at(handleEdges(x,     input.width()), handleEdges(y + 1, input.height()));
            const double bottomRight  = intensity.at(handleEdges(x + 1, input.width()), handleEdges(y + 1, input.height()));
            const double bottom       = intensity.at(handleEdges(x + 1, input.width()), handleEdges(y,     input.height()));
            const double bottomLeft   = intensity.at(handleEdges(x + 1, input.width()), handleEdges(y - 1, input.height()));
            const double left         = intensity.at(handleEdges(x,     input.width()), handleEdges(y - 1, input.height()));

            const double convolution_kernel[3][3] = {{topLeft, top, topRight},
                                               {left, 0.0, right},
                                               {bottomLeft, bottom, bottomRight}};

            QVector3D normal(0, 0, 0);

            if(kernel == SOBEL)
                normal = sobel(convolution_kernel, strengthInv);
            else if(kernel == PREWITT)
                normal = prewitt(convolution_kernel, strengthInv);

            scanline[x] = qRgb(mapComponent(normal.x()), mapComponent(normal.y()), mapComponent(normal.z()));
        }
    }
    
    if(keepLargeDetail) {
        //generate a second normalmap from a downscaled input image, then mix both normalmaps
        
        int largeDetailMapWidth = (int) (((double)input.width() / 100.0) * largeDetailScale);
        int largeDetailMapHeight = (int) (((double)input.height() / 100.0) * largeDetailScale);
        
        //create downscaled version of input
        QImage inputScaled = input.scaled(largeDetailMapWidth, largeDetailMapHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        //compute downscaled normalmap
        QImage largeDetailMap = calculateNormalmap(inputScaled, kernel, largeDetailHeight, invert, tileable, false, 0, 0.0);
        //scale map up
        largeDetailMap = largeDetailMap.scaled(input.width(), input.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        
        #pragma omp parallel for  // OpenMP
        //mix the normalmaps
        for(int y = 0; y < input.height(); y++) {
            QRgb *scanlineResult = (QRgb*) result.scanLine(y);
            QRgb *scanlineLargeDetail = (QRgb*) largeDetailMap.scanLine(y);

            for(int x = 0; x < input.width(); x++) {
                const QRgb colorResult = scanlineResult[x];
                const QRgb colorLargeDetail = scanlineLargeDetail[x];

                const int r = blendSoftLight(qRed(colorResult), qRed(colorLargeDetail));
                const int g = blendSoftLight(qGreen(colorResult), qGreen(colorLargeDetail));
                const int b = blendSoftLight(qBlue(colorResult), qBlue(colorLargeDetail));

                scanlineResult[x] = qRgb(r, g, b);
            }
        }
    }

    return result;
}

QVector3D NormalmapGenerator::sobel(const double convolution_kernel[3][3], double strengthInv) const {
    const double top_side    = convolution_kernel[0][0] + 2.0 * convolution_kernel[0][1] + convolution_kernel[0][2];
    const double bottom_side = convolution_kernel[2][0] + 2.0 * convolution_kernel[2][1] + convolution_kernel[2][2];
    const double right_side  = convolution_kernel[0][2] + 2.0 * convolution_kernel[1][2] + convolution_kernel[2][2];
    const double left_side   = convolution_kernel[0][0] + 2.0 * convolution_kernel[1][0] + convolution_kernel[2][0];

    const double dY = right_side - left_side;
    const double dX = bottom_side - top_side;
    const double dZ = strengthInv;

    return QVector3D(dX, dY, dZ).normalized();
}

QVector3D NormalmapGenerator::prewitt(const double convolution_kernel[3][3], double strengthInv) const {
    const double top_side    = convolution_kernel[0][0] + convolution_kernel[0][1] + convolution_kernel[0][2];
    const double bottom_side = convolution_kernel[2][0] + convolution_kernel[2][1] + convolution_kernel[2][2];
    const double right_side  = convolution_kernel[0][2] + convolution_kernel[1][2] + convolution_kernel[2][2];
    const double left_side   = convolution_kernel[0][0] + convolution_kernel[1][0] + convolution_kernel[2][0];

    const double dY = right_side - left_side;
    const double dX = top_side - bottom_side;
    const double dZ = strengthInv;

    return QVector3D(dX, dY, dZ).normalized();
}

int NormalmapGenerator::handleEdges(int iterator, int maxValue) const {
    if(iterator >= maxValue) {
        //move iterator from end to beginning + overhead
        if(tileable)
            return maxValue - iterator;
        else
            return maxValue - 1;
    }
    else if(iterator < 0) {
        //move iterator from beginning to end - overhead
        if(tileable)
            return maxValue + iterator;
        else
            return 0;
    }
    else {
        return iterator;
    }
}

//transform -1..1 to 0..255
int NormalmapGenerator::mapComponent(double value) const {
    return (value + 1.0) * (255.0 / 2.0);
}

//uses a similar algorithm like "soft light" in PS, 
//see http://www.michael-kreil.de/algorithmen/photoshop-layer-blending-equations/index.php
int NormalmapGenerator::blendSoftLight(int color1, int color2) const {
    const float a = color1;
    const float b = color2;
    
    if(2.0f * b < 255.0f) {
        return (int) (((a + 127.5f) * b) / 255.0f);
    }
    else {
        return (int) (255.0f - (((382.5f - a) * (255.0f - b)) / 255.0f));
    }
}
