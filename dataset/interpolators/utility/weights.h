#pragma once
#include <algorithm>
#include <iterator>
#include <numeric>

namespace Interpolator::Utility
{
    template<typename Vector, typename DemoList, typename WeightList>
    Vector weighted_sum(const DemoList& demo, const WeightList& weight, Vector v)
    {
        return std::transform_reduce(std::begin(demo), std::end(demo), std::begin(weight), v,
                [](const Vector& v1, const Vector& v2){ return v1 + v2; },
                [](const auto& demo, const auto& weight){ return destination(demo) * weight; });
    }

    template<typename Scalar, typename WeightList>
    Scalar sum_of_weights(WeightList& weight)
    {
        return std::accumulate(std::begin(weight), std::end(weight), (Scalar)0);
    }

    template<typename Scalar, typename Vector, typename DemoList, typename WeightList>
    Vector normalized_weighted_sum(const DemoList& demo, const WeightList& weight, Vector v)
    {
        return (Scalar)(1 / sum_of_weights<Scalar>(weight)) * weighted_sum(demo, weight, v);
    }
}
