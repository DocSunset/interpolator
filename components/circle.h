#pragma once
#include "position.h"
#include "color.h"

namespace Component
{
    // draw a visible circle
    struct Circle
    {
        float radius; // pixels
        float center_point[2];
        float fill_color[4];
    };


}
