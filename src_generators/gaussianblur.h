#ifndef GAUSSIANBLUR_H
#define GAUSSIANBLUR_H

#include "intensitymap.h"

class GaussianBlur
{
public:
    GaussianBlur();
    IntensityMap calculate(IntensityMap map, int radius);

private:
    double* kernel;

    double* computeKernel(int radius);
};

#endif // GAUSSIANBLUR_H
