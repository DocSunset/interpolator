#include "color.h"
#include "utility/random.h"

namespace Component
{
    Color Color::Random()
    {
        using Utility::nrandf;
        return {nrandf(),nrandf(),nrandf(),1.0f};
    }

    bool operator==(const Color& a, const Color& b)
    {
        if (a.r != b.r) return false;
        if (a.g != b.g) return false;
        if (a.b != b.b) return false;
        if (a.a != b.a) return false;
        return true;
    }

    bool operator!=(const Color& a, const Color& b)
    {
        return not (a == b);
    }
}
