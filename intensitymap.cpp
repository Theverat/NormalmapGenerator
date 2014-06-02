#include "intensitymap.h"

IntensityMap::IntensityMap()
{

}

IntensityMap::IntensityMap(QImage rgbImage)
{
    for(int y = 0; y < rgbImage.height(); y++) {
        std::vector<double> row;
        for(int x = 0; x < rgbImage.width(); x++) {
            double r = QColor(rgbImage.pixel(x, y)).red();
            double g = QColor(rgbImage.pixel(x, y)).green();
            double b = QColor(rgbImage.pixel(x, y)).blue();

            double average = (r + g + b) / 3.0;
            row.push_back(average);
        }
        this->map.push_back(row);
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
    for(int y = 0; y < this->map.size(); y++) {
        for(int x = 0; x < this->map.at(0).size(); x++) {
            double inverted = 1.0 - this->map.at(y).at(x);
            this->map.at(y).at(x) = inverted;
        }
    }
}
