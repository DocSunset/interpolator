#pragma once

namespace Component
{
    struct Position
    {
        float x;
        float y;
        static Position Random();
    };

    float distance(const Position& p1, const Position& p2);
}
