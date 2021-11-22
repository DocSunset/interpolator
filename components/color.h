#pragma once

namespace Component
{
    struct Color
    {
        float r;
        float g;
        float b;
        float a;
        static Color Random();
    };

    bool operator==(const Color&, const Color&);
    bool operator!=(const Color&, const Color&);
}
