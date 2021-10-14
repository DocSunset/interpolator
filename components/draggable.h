#pragma once

#include "position.h"
#include "flag.h"

namespace Component
{
    struct Selected : public Flag
    {
        // selection group ?
        bool _touched;
    };

    struct SelectionHovered : public Flag
    {
    };

    struct Draggable
    {
        float radius;
        Position start;
        Position previous;
        Position current;
        Position delta;
    };
}