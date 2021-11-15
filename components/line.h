#pragma once

namespace Component
{
    struct Line
    {
        enum class Cap : int
        {
            None = 0,
            Round = 1,
        };

        float color[4];
        float border[4];
        float start_position[2];
        float end_position[2];
        float line_thickness;
        float border_thickness;
        float border_transition;
        float blur_radius;
        Cap cap;
        int _id; // this is ignored in components created by the user
    };
}
