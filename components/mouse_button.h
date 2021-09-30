#pragma once

#include "position.h"

namespace Component
{
    struct _MouseButton
    {
        bool pressed;
        int clicks;

        Position down_position;
        Position up_position;
    };

    struct LeftMouseButton : public _MouseButton {};
    struct RightMouseButton : public _MouseButton {};
    struct MiddleMouseButton : public _MouseButton {};
}
