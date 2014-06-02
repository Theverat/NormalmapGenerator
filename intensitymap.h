#ifndef INTENSITYMAP_H
#define INTENSITYMAP_H

#include <QImage>
#include <QColor>

class IntensityMap
{
public:
    IntensityMap();
    IntensityMap(QImage rgbImage);
    double at(int x, int y);
    int getWidth();
    int getHeight();
    void invert();

private:
    std::vector< std::vector<double> > map;
};

#endif // INTENSITYMAP_H
