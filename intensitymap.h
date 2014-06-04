#ifndef INTENSITYMAP_H
#define INTENSITYMAP_H

#include <QImage>
#include <QColor>

class IntensityMap
{
public:
    enum Mode {
        AVERAGE,
        MAX
    };

    IntensityMap();
    IntensityMap(QImage rgbImage, Mode mode, bool useRed = true, bool useGreen = true, bool useBlue = true, bool useAlpha = false);
    double at(int x, int y);
    int getWidth();
    int getHeight();
    void invert();
    QImage convertToQImage();

private:
    std::vector< std::vector<double> > map;
};

#endif // INTENSITYMAP_H
