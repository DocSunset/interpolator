#pragma once
#include <algorithm>
#include <iterator>
#include <numeric>

namespace Interpolator::Utility
{
    template<typename Vector, typename DemoList, typename WeightList>
    Vector weighted_sum(const DemoList& demo, const WeightList& weight, Vector v)
    {
        return std::inner_product(std::begin(demo), std::end(demo), std::begin(weight), v);
    }

    template<typename Scalar, typename WeightList>
    Scalar sum_of_weights(WeightList& weight)
    {
        return std::accumulate(std::begin(weight), std::end(weight), (Scalar)0);
    }

    template<typename Scalar, typename Vector, typename DemoList, typename WeightList>
    Vector normalized_weighted_sum(const DemoList& demo, const WeightList& weight, Vector v)
    {
        return (Scalar)(1 / sum_of_weights(weight)) * weighted_sum(demo, weight, v);
    }
}
