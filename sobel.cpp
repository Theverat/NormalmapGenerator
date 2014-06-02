#include "sobel.h"
#include <math.h>

Sobel::Sobel()
{
}

QImage Sobel::calculateNormalmap(QImage input, double strength, bool inverted, bool tileable) {
    this->tileable = tileable;

    this->intensity = IntensityMap(input);
    if(inverted)
        intensity.invert();

    QImage result(input.width(), input.height(), QImage::Format_RGB888);

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

            //sobel filter
            double dY = (topRight + 2.0 * right + bottomRight) - (topLeft + 2.0 * left + bottomLeft);
            double dX = (bottomLeft + 2.0 * bottom + bottomRight) - (topLeft + 2.0 * top + topRight);
            double dZ = 300.0 / strength;

            QVector3D normal(dX, dY, dZ);
            normal.normalize();

            QColor normalAsRgb(mapComponent(normal.x()), mapComponent(normal.y()), mapComponent(normal.z()));
            result.setPixel(x, y, normalAsRgb.rgb());
        }
    }

    return result;
}

int Sobel::handleEdges(int iterator, int max) {
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
int Sobel::mapComponent(double value) {
    return (value + 1.0) * (255.0 / 2.0);
}
