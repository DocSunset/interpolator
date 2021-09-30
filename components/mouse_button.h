#pragma once

#include "position.h"

namespace Component
{
    struct _MouseButton
    {
        bool pressed;
        int clicks;

        Position position;
    };

    struct LeftMouseButton : public _MouseButton {};
    struct RightMouseButton : public _MouseButton {};
    struct MiddleMouseButton : public _MouseButton {};
}
