#pragma once

namespace Component
{
    struct Cursor
    {
        unsigned int radius = 30;
        bool hidden = false;
    };

    enum class CursorMode
    {
        Edit,
        Interact,
    };
}
