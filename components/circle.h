#pragma once
#include "position.h"
#include "color.h"

namespace Component
{
    // draw a visible circle
    struct Circle
    {
        float color[4];
        float border[4];
        float position[2];
        float radius;
        float border_thickness;
        float border_transition = 2;
        float blur_radius = 2;
    };


}
