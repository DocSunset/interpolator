#pragma once
#include "flag.h"
#include "position.h"
#include "color.h"

namespace Component
{
    struct ManualVis : public Flag {};
    struct ManualPosition { Position value; };
    struct ManualColor { Color value; };
}
