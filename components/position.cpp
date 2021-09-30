#include "position.h"

#include <cmath>
#include "utility/random.h"

namespace Component
{
    Position Position::Random()
    {
        return {Utility::brandf() * 100, Utility::brandf() * 100};
    }

    float distance(const Position& p1, const Position& p2)
    {
        Position diff{p1.x - p2.x, p1.y - p2.y};
        return std::sqrt(diff.x * diff.x + diff.y * diff.y);
    }
}
