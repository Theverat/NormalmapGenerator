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
