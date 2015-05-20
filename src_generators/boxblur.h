#ifndef BOXBLUR_H
#define BOXBLUR_H

#include "intensitymap.h"

class BoxBlur
{
public:
    BoxBlur();
    IntensityMap calculate(IntensityMap input, int radius, bool tileable);

private:
    int handleEdges(int iterator, int max, bool tileable);
};

#endif // BOXBLUR_H
