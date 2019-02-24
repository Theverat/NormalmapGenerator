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

#include "specularmapgenerator.h"
#include <QColor>

SpecularmapGenerator::SpecularmapGenerator(IntensityMap::Mode mode, double redMultiplier, double greenMultiplier, double blueMultiplier, double alphaMultiplier)
{
    this->mode = mode;
    this->redMultiplier = redMultiplier;
    this->greenMultiplier = greenMultiplier;
    this->blueMultiplier = blueMultiplier;
    this->alphaMultiplier = alphaMultiplier;
}

QImage SpecularmapGenerator::calculateSpecmap(const QImage &input, double scale, double contrast) {
    QImage result(input.width(), input.height(), QImage::Format_ARGB32);
    
    //generate contrast lookup table
    unsigned short contrastLookup[256];
    double newValue = 0;
    
    for(int i = 0; i < 256; i++) {
        newValue = (double)i;
        newValue /= 255.0;
        newValue -= 0.5;
        newValue *= contrast;
        newValue += 0.5;
        newValue *= 255;
    
        if(newValue < 0)
            newValue = 0;
        if(newValue > 255)
            newValue = 255;
        
        contrastLookup[i] = (unsigned short)newValue;
    }
    
    // This is outside of the loop because the multipliers are the same for every pixel
    double multiplierSum = ((redMultiplier != 0.0) + (greenMultiplier != 0.0) +
            (blueMultiplier != 0.0) + (alphaMultiplier != 0.0));
    if(multiplierSum == 0.0)
        multiplierSum = 1.0;

    #pragma omp parallel for  // OpenMP
    //for every row of the image
    for(int y = 0; y < result.height(); y++) {
        QRgb *scanline = (QRgb*) result.scanLine(y);

        //for every column of the image
        for(int x = 0; x < result.width(); x++) {
            double intensity = 0.0;

            const QColor pxColor = QColor(input.pixel(x, y));

            const double r = pxColor.redF() * redMultiplier;
            const double g = pxColor.greenF() * greenMultiplier;
            const double b = pxColor.blueF() * blueMultiplier;
            const double a = pxColor.alphaF() * alphaMultiplier;

            if(mode == IntensityMap::AVERAGE) {
                //take the average out of all selected channels
                intensity = (r + g + b + a) / multiplierSum;
            }
            else if(mode == IntensityMap::MAX) {
                //take the maximum out of all selected channels
                const double tempMaxRG = std::max(r, g);
                const double tempMaxBA = std::max(b, a);
                intensity = std::max(tempMaxRG, tempMaxBA);
            }

            //apply scale (brightness)
            intensity *= scale;

            //clamp
            if(intensity > 1.0)
                intensity = 1.0;

            //convert intensity to the 0-255 range
            int c = (int)(255.0 * intensity);
            
            //apply contrast
            c = (int)contrastLookup[c];
            
            //write color into image pixel
            scanline[x] = qRgba(c, c, c, pxColor.alpha());
        }
    }

    return result;
}
