#include "normalmapgenerator.h"
#include <math.h>
#include <iostream>

NormalmapGenerator::NormalmapGenerator(IntensityMap::Mode mode, bool useRed, bool useGreen, bool useBlue, bool useAlpha)
{
    this->mode = mode;
    this->useRed = useRed;
    this->useGreen = useGreen;
    this->useBlue = useBlue;
    this->useAlpha = useAlpha;
}

QImage NormalmapGenerator::calculateNormalmap(QImage input, Kernel kernel, double strength, bool invert, bool tileable) {
    this->tileable = tileable;

    this->intensity = IntensityMap(input, mode, useRed, useGreen, useBlue, useAlpha);
    if(invert)
        intensity.invert();

    QImage result(input.width(), input.height(), QImage::Format_ARGB32);

    //code from http://stackoverflow.com/a/2368794
    for(int y = 0; y < input.height(); y++) {
        for(int x = 0; x < input.width(); x++) {

            double topLeft = intensity.at(handleEdges(x - 1, input.width()), handleEdges(y - 1, input.height()));
            double top = intensity.at(handleEdges(x - 1, input.width()), handleEdges(y, input.height()));
            double topRight = intensity.at(handleEdges(x - 1, input.width()), handleEdges(y + 1, input.height()));
            double right = intensity.at(handleEdges(x, input.width()), handleEdges(y + 1, input.height()));
            double bottomRight = intensity.at(handleEdges(x + 1, input.width()), handleEdges(y + 1, input.height()));
            double bottom = intensity.at(handleEdges(x + 1, input.width()), handleEdges(y, input.height()));
            double bottomLeft = intensity.at(handleEdges(x + 1, input.width()), handleEdges(y - 1, input.height()));
            double left = intensity.at(handleEdges(x, input.width()), handleEdges(y - 1, input.height()));

            double tmp[] = {topLeft, top, topRight, right, bottomRight, bottom, bottomLeft, left};
            std::vector<double> neighbours(tmp, tmp + sizeof(tmp) / sizeof(tmp[0]));
            //                                  0     1      2        3       4           5          6      7
            //sobel filter
            //double dY = (topRight + 2.0 * right + bottomRight) - (topLeft + 2.0 * left + bottomLeft);
            //double dX = (bottomLeft + 2.0 * bottom + bottomRight) - (topLeft + 2.0 * top + topRight);
            //double dZ = 300.0 / strength;

            //QVector3D normal(dX, dY, dZ);
            //normal.normalize();

            QVector3D normal(0, 0, 0);

            if(kernel == SOBEL)
                normal = sobel(neighbours, strength);

            QColor normalAsRgb(mapComponent(normal.x()), mapComponent(normal.y()), mapComponent(normal.z()));
            result.setPixel(x, y, normalAsRgb.rgb());
        }
    }

    return result;
}

QVector3D NormalmapGenerator::sobel(std::vector<double> neighbours, double strength) {
    double dY = (neighbours.at(2) + 2.0 * neighbours.at(3) + neighbours.at(4)) - (neighbours.at(0) + 2.0 * neighbours.at(7) + neighbours.at(6));
    double dX = (neighbours.at(6) + 2.0 * neighbours.at(5) + neighbours.at(4)) - (neighbours.at(0) + 2.0 * neighbours.at(1) + neighbours.at(2));
    double dZ = 1.0 / strength;

    return QVector3D(dX, dY, dZ).normalized();
}

int NormalmapGenerator::handleEdges(int iterator, int max) {
    if(iterator >= max) {
        //move iterator from end to beginning + overhead
        if(tileable)
            return max - iterator;
        else
            return max - 1;
    }
    else if(iterator < 0) {
        //move iterator from beginning to end - overhead
        if(tileable)
            return max + iterator;
        else
            return 0;
    }
    else {
        return iterator;
    }
}

//transform -1..1 to 0..255
int NormalmapGenerator::mapComponent(double value) {
    return (value + 1.0) * (255.0 / 2.0);
}
