#pragma once

#include "position.h"
#include "flag.h"
#include <typeindex>

namespace Component
{

    // boolean value represents whether or not the entity is already selected
    struct Selectable : public Flag
    {
        enum class Group
        {
            Demo,
            Knob,
            Cursor,
            All,
        } group;
        bool _touched;
    };

    struct Selected {}; // tag struct to signal that a component is selected
    struct Grabbed {}; // tag struct to signal that a component is grabbed for dragging

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
