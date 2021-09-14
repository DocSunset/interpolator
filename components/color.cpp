#include "color.h"
#include "utility/random.h"

namespace Component
{
    Color Color::Random()
    {
        using Utility::nrandf;
        return {nrandf(),nrandf(),nrandf(),nrandf()};
    }
}
