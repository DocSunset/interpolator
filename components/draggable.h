#pragma once

#include "position.h"
#include "flag.h"

namespace Component
{
    // boolean value represents whether or not the entity is already selected
    struct Selectable : public Flag
    {
        // selection group ?
        bool _touched;
    };

    struct Selected {}; // tag struct to signal that a component is selected

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
