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
        } group = Group::All;
        bool _touched = false;
    };

    struct Selected {}; // tag struct to signal that a component is selected
    struct Grabbed {}; // tag struct to signal that a component is grabbed for dragging

    struct SelectionHovered : public Flag
    {
    };

    struct Draggable
    {
        float radius;
        Position start = Position::Zero();
        Position previous = Position::Zero();
        Position current = Position::Zero();
        Position delta = Position::Zero();
    };
}
