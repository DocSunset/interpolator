#pragma once

namespace Component
{
    struct Position;

    Position operator*(const Position& p, float scalar);
    Position operator*(float scalar, const Position& p);
    Position operator*(const Position& lhs, const Position& rhs); // dot product
    Position operator-(const Position& lhs, const Position& rhs);
    Position operator+(const Position& lhs, const Position& rhs);
    float norm_squared(const Position& p);
    float norm(const Position& p);
    float distance(const Position& p1, const Position& p2);

    struct Position
    {
        float x;
        float y;

        static Position Random();
        static Position Zero();

        bool operator==(const Position& other) const;
        bool operator!=(const Position& other) const;

        Position& operator+=(const Position& other)
        {
            *this = *this + other;
            return *this;
        }

        Position& operator-=(const Position& other)
        {
            *this = *this - other;
            return *this;
        }
    };
}
