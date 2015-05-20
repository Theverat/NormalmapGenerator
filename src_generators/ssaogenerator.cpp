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

#include "ssaogenerator.h"
#include <QVector3D>
#include <QMatrix4x4>
#include <QColor>

SsaoGenerator::SsaoGenerator() {
}

QImage SsaoGenerator::calculateSsaomap(QImage normalmap, QImage depthmap, float radius, unsigned int kernelSamples, unsigned int noiseSize) {
    QImage result(normalmap.width(), normalmap.height(), QImage::Format_ARGB32);
    std::vector<QVector3D> kernel = generateKernel(kernelSamples);
    std::vector<QVector3D> noiseTexture = generateNoise(noiseSize);

    #pragma omp parallel for  // OpenMP
    for(int y = 0; y < normalmap.height(); y++) {
        QRgb *scanline = (QRgb*) result.scanLine(y);

        for(int x = 0; x < normalmap.width(); x++) {
            QVector3D origin(x, y, 1.0);

            float r = QColor(normalmap.pixel(x, y)).redF();
            float g = QColor(normalmap.pixel(x, y)).greenF();
            float b = QColor(normalmap.pixel(x, y)).blueF();
            QVector3D normal(r, g, b);

            //reorient the kernel along the normal
            //get random vector from noise texture
            QVector3D randVec = noiseTexture.at((int)random(0, noiseTexture.size() - 1));

            QVector3D tangent = (randVec - normal * QVector3D::dotProduct(randVec, normal)).normalized();
            QVector3D bitangent = QVector3D::crossProduct(normal, tangent);
            QMatrix4x4 transformMatrix = QMatrix4x4(tangent.x(), bitangent.x(), normal.x(), 0,
                                                    tangent.y(), bitangent.y(), normal.y(), 0,
                                                    tangent.z(), bitangent.z(), normal.z(), 0,
                                                    0, 0, 0, 0);

            float occlusion = 0.0;

            for(unsigned int i = 0; i < kernel.size(); i++) {
                //get sample position
                QVector3D sample = transformMatrix * kernel[i];
                sample = (sample * radius) + origin;

                //get sample depth
                float sampleDepth = QColor(depthmap.pixel(x, y)).redF();

                //range check and accumulate
                float rangeCheck = fabs(origin.z() - sampleDepth) < radius ? 1.0 : 0.0;
                occlusion += (sampleDepth <= sample.z() ? 1.0 : 0.0) * rangeCheck;
            }

            //normalize and invert occlusion factor
            occlusion = occlusion / kernel.size();

            //convert occlusion to the 0-255 range
            int c = (int)(255.0 * occlusion);
            //write result
            scanline[x] = qRgba(c, c, c, 255);
        }
    }

    return result;
}

std::vector<QVector3D> SsaoGenerator::generateKernel(unsigned int size) {
    std::vector<QVector3D> kernel = std::vector<QVector3D>(size, QVector3D());

    //generate hemisphere
    for (unsigned int i = 0; i < size; i++) {
        //points on surface of a hemisphere
        kernel[i] = QVector3D(random(-1.0, 1.0), random(-1.0, 1.0), random(0.0, 1.0)).normalized();
        //scale into the hemisphere
        kernel[i] *= random(0.0, 1.0);
        //generate falloff
        float scale = float(i) / float(size);
        scale = lerp(0.1f, 1.0f, scale * scale);
        kernel[i] *= scale;
    }

    return kernel;
}

std::vector<QVector3D> SsaoGenerator::generateNoise(unsigned int size) {
    std::vector<QVector3D> noise = std::vector<QVector3D>(size, QVector3D());

    for (unsigned int i = 0; i < size; i++) {
        noise[i] = QVector3D(random(-1.0, 1.0), random(-1.0, 1.0), 0.0).normalized();
    }

    return noise;
}

double SsaoGenerator::random(double min, double max) {
    double f = (double)rand() / RAND_MAX;
    return min + f * (max - min);
}

float SsaoGenerator::lerp(float v0, float v1, float t) {
    return (1 - t) * v0 + t * v1;
}
