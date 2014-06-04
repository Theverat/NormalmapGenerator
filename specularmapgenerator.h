#ifndef SPECULARMAPGENERATOR_H
#define SPECULARMAPGENERATOR_H

#include "intensitymap.h"

class SpecularmapGenerator
{
public:
    SpecularmapGenerator(IntensityMap::Mode mode, int redMultiplier, int greenMultiplier, int blueMultiplier, int alphaMultiplier);
    QImage calculateSpecmap(QImage input, double scale);

private:
    IntensityMap intensity;
    double redMultiplier, greenMultiplier, blueMultiplier, alphaMultiplier;
    IntensityMap::Mode mode;
};

#endif // SPECULARMAPGENERATOR_H
