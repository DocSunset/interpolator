#pragma once

namespace Utility
{
    // generate a random normalized float from 0 to 1
    float nrandf();

    // generate a random bi-polar float from -1 to 1
    float brandf(); 

    // generate a random float within the range [min, max)
    // note: if max < min the range will be flipped, i.e. [max, min)
    float rrandf(float min, float max);
}
