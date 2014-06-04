#ifndef SOBEL_H
#define SOBEL_H

#include <QImage>
#include <QVector3D>
#include "intensitymap.h"

class NormalmapGenerator
{
public:
    enum Kernel {
        SOBEL
    };

    NormalmapGenerator(IntensityMap::Mode mode, bool useRed, bool useGreen, bool useBlue, bool useAlpha);
    QImage calculateNormalmap(QImage input, Kernel kernel, double strength = 2.0, bool invert = false, bool tileable = true);

private:
    IntensityMap intensity;
    bool tileable;
    bool useRed, useGreen, useBlue, useAlpha;
    IntensityMap::Mode mode;

    int handleEdges(int iterator, int max);
    int mapComponent(double value);
    QVector3D sobel(std::vector<double> neighbours, double strength);
};

#endif // SOBEL_H
