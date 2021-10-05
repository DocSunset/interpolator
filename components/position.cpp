#include "position.h"

#include <cmath>
#include "utility/random.h"

namespace Component
{
    Position operator*(const Position& p, float scalar)
    {
        return {p.x * scalar, p.y * scalar};
    }

    Position operator*(float scalar, const Position& p)
    {
        return {scalar * p.x, scalar * p.y};
    }

    Position operator*(const Position& lhs, const Position& rhs)
    {
        return {lhs.x * rhs.x, lhs.y * rhs.y};
    }

    Position operator-(const Position& lhs, const Position& rhs)
    {
        return {lhs.x - rhs.x, lhs.y - rhs.y};
    }

    Position operator+(const Position& lhs, const Position& rhs)
    {
        return {lhs.x + rhs.x, lhs.y + rhs.y};
    }

    float norm_squared(const Position& p)
    {
        auto dot = p * p;
        return dot.x + dot.y;
    }

    float norm(const Position& p)
    {
        return std::sqrt(norm_squared(p));
    }

    float distance(const Position& p1, const Position& p2)
    {
        return norm(p1 - p2);
    }

    Position Position::Random()
    {
        return {Utility::brandf() * 100, Utility::brandf() * 100};
    }

    Position Position::Zero()
    {
        return {0, 0};
    }

    bool Position::operator==(const Position& other) const
    {
        if (x == other.x && y == other.y) return true;
        else return false;
    }

    bool Position::operator!=(const Position& other) const
    {
        if (x != other.x || y != other.y) return true;
        else return false;
    }
}
