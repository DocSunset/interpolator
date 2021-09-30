#pragma once

#include "position.h"

namespace Component
{
    // motion of the mouse when no buttons are actuated
 
    struct MouseMotion
    {
        Position position;
        Position delta;
    };
}
