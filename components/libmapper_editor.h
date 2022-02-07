#pragma once
#include "components/demo.h"

namespace Component
{
    struct Grab
    {
        enum class State
        {
            Hovering,
            Grabbing,
            Dragging,
            Dropping,
        } state = State::Hovering;
        Demo::Source position = Demo::Source::Zero();
    };

    struct LibmapperInsertDemo {};
    struct LibmapperDeleteDemo {};
    struct LibmapperHovered {};
}
