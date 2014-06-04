#include "intensitymap.h"
#include <iostream>

IntensityMap::IntensityMap() {

}

IntensityMap::IntensityMap(QImage rgbImage, Mode mode, bool useRed, bool useGreen, bool useBlue, bool useAlpha)
{
    map = std::vector< std::vector<double> >(rgbImage.height(), std::vector<double>(rgbImage.width(), 0.0));

    #pragma omp parallel for
    //for every row of the image
    for(int y = 0; y < rgbImage.height(); y++) {
        //for every column of the image
        for(int x = 0; x < rgbImage.width(); x++) {
            double r, g, b, a;
            double intensity = 0.0;

            r = QColor(rgbImage.pixel(x, y)).redF();
            g = QColor(rgbImage.pixel(x, y)).greenF();
            b = QColor(rgbImage.pixel(x, y)).blueF();
            a = QColor(rgbImage.pixel(x, y)).alphaF();

            if(mode == AVERAGE) {
                //take the average out of all selected channels
                int num_channels = 0;

                if(useRed) {
                    intensity += r;
                    num_channels++;
                }
                if(useGreen) {
                    intensity += g;
                    num_channels++;
                }
                if(useBlue) {
                    intensity += b;
                    num_channels++;
                }
                if(useAlpha) {
                    intensity += a;
                    num_channels++;
                }

                if(num_channels != 0)
                    intensity /= num_channels;
                else
                    intensity = 0.0;
            }
            else if(mode == MAX) {
                //take the maximum out of all selected channels
                double tempR = r;
                double tempG = g;
                double tempB = b;
                double tempA = a;

                if(!useRed)
                    tempR = 0.0;
                if(!useGreen)
                    tempG = 0.0;
                if(!useBlue)
                    tempB = 0.0;
                if(!useAlpha)
                    tempA = 0.0;

                double tempMaxRG = std::max(tempR, tempG);
                double tempMaxBA = std::max(tempB, tempA);
                intensity = std::max(tempMaxRG, tempMaxBA);
            }

            //add resulting pixel intensity to intensity map
            this->map.at(y).at(x) = intensity;
        }
    }
}

double IntensityMap::at(int x, int y) {
    return this->map.at(y).at(x);
}

int IntensityMap::getWidth() {
    return this->map.at(0).size();
}

int IntensityMap::getHeight() {
    return this->map.size();
}

void IntensityMap::invert() {
    for(unsigned int y = 0; y < this->map.size(); y++) {
        for(unsigned int x = 0; x < this->map.at(0).size(); x++) {
            double inverted = 1.0 - this->map.at(y).at(x);
            this->map.at(y).at(x) = inverted;
        }
    }
}

QImage IntensityMap::convertToQImage() {
    QImage result(this->getWidth(), this->getHeight(), QImage::Format_ARGB32);

    for(int y = 0; y < this->getHeight(); y++) {
        for(int x = 0; x < this->getWidth(); x++) {
            int c = 255 * map.at(y).at(x);
            result.setPixel(x, y, QColor(c, c, c, c).rgba());
        }
    }

    return result;
}
