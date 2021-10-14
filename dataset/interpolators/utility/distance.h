#pragma once

namespace Interpolator::Utility
{
    template<typename Vector>
    auto distance_to(const Vector& q)
    {
        return [&](const auto& demo)
        {
            return norm(source(demo) - q);
        };
    }
}
