#pragma once

namespace Component
{
    struct Position
    {
        float x;
        float y;
        static Position Random();
        bool operator==(const Position& other) const;
        bool operator!=(const Position& other) const;
    };

    float distance(const Position& p1, const Position& p2);
}
