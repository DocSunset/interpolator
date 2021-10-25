#pragma once

#include "position.h"

namespace Component
{
    struct MouseMotion
    {
        Position position;
        Position delta;
    };
}
