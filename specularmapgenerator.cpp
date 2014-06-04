#include "specularmapgenerator.h"

SpecularmapGenerator::SpecularmapGenerator(IntensityMap::Mode mode, int redMultiplier, int greenMultiplier, int blueMultiplier, int alphaMultiplier)
{
    this->mode = mode;
    this->redMultiplier = redMultiplier;
    this->greenMultiplier = greenMultiplier;
    this->blueMultiplier = blueMultiplier;
    this->alphaMultiplier = alphaMultiplier;
}

QImage SpecularmapGenerator::calculateSpecmap(QImage input, double scale) {
    QImage result(input.width(), input.height(), QImage::Format_ARGB32);

    //for every row of the image
    for(int y = 0; y < result.height(); y++) {
        //for every column of the image
        for(int x = 0; x < result.width(); x++) {
            double r, g, b, a;
            double intensity = 0.0;

            QColor pxColor = QColor(input.pixel(x, y));

            r = pxColor.redF() * redMultiplier;
            g = pxColor.greenF() * greenMultiplier;
            b = pxColor.blueF() * blueMultiplier;
            a = pxColor.alphaF() * alphaMultiplier;

            if(mode == IntensityMap::AVERAGE) {
                //take the average out of all selected channels
                double multiplierSum = (redMultiplier + greenMultiplier + blueMultiplier + alphaMultiplier);

                if(multiplierSum == 0.0)
                    multiplierSum = 1.0;

                intensity = (r + g + b + a) / multiplierSum;
            }
            else if(mode == IntensityMap::MAX) {
                //take the maximum out of all selected channels
                double tempMaxRG = std::max(r, g);
                double tempMaxBA = std::max(b, a);
                intensity = std::max(tempMaxRG, tempMaxBA);
            }

            intensity *= scale;

            if(intensity > 1.0)
                intensity = 1.0;

            //write color into image pixel
            int c = 255 * intensity;
            result.setPixel(x, y, QColor(c, c, c, c).rgba());
        }
    }

    return result;
}
