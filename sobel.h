#ifndef SOBEL_H
#define SOBEL_H

#include <QImage>
#include <QVector3D>
#include "intensitymap.h"

class Sobel
{
public:
    Sobel();
    QImage calculateNormalmap(QImage input, double strength = 2.0, bool inverted = false, bool tileable = true);

private:
    IntensityMap intensity;
    bool tileable;

    int handleEdges(int iterator, int max);
    int mapComponent(double value);
};

#endif // SOBEL_H
