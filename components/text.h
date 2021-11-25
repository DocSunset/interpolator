#pragma once
#include <string>
#include "color.h"

namespace Component
{
    struct Text
    {
        std::string string;

        Color color;

        // in pixels
        float left;
        float bottom;
        float right;
        float top;
        float font_size = 20;
    };
}
