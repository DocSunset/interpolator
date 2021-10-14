#pragma once

namespace Interpolator::Utility
{
    template<typename Demo, typename OutputAttribute, typename Vector>
    auto distance_to(const Vector& q)
    {
        return [&](const Demo& demo)
        {
            return norm(source(demo) - q);
        };
    }
}
