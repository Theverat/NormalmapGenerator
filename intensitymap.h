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
    IntensityMap(QImage rgbImage, Mode mode, bool useRed = true, bool useGreen = true, bool useBlue = true, bool useAlpha = false);
    double at(int x, int y);
    void setValue(int x, int y, double value);
    int getWidth();
    int getHeight();
    void invert();
    QImage convertToQImage();

private:
    std::vector< std::vector<double> > map;
};

#endif // INTENSITYMAP_H
