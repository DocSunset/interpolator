#pragma once
#include <cmath>
#include <algorithm>
#include <limits>
#include <numeric>
#include "utility/distance.h"

namespace Interpolator
{
    template<typename Scalar>
    Scalar circle_circle_intersection_area(
            const Scalar& R, 
            const Scalar& r, 
            const Scalar& d)
    {
        Scalar d2 = d * d;
        Scalar r2 = r * r;
        Scalar R2 = R * R;
        Scalar two_d = (Scalar)2 * d;
        Scalar arg1 = (d2 + r2 - R2)/(two_d * r);
        Scalar arg2 = (d2 + R2 - r2)/(two_d * R);
        Scalar arg3 = (-d+r+ R) * (d+r-R) * (d-r+R) * (d+r+R);
        Scalar a, b, c;
        if constexpr (std::is_same_v<Scalar, float>)
        {
            if (arg1 > 1.0f) a = 0;
            else a = r2 * std::acos(arg1);
            if (arg2 > 1.0f) b = 0;
            else b = R2 * std::acos(arg2);
            if (arg3 < 0.0f) c = 0;
            else c = std::sqrt(arg3) / 2.0f;
        }
        else
        {
            a = r2 * std::acos(arg1);
            b = R2 * std::acos(arg2);
            c = std::sqrt(arg3) / (Scalar)2.0;
        }
        return a + b - c;
    }

    template<typename Scalar>
    Scalar circle_area(const Scalar& r)
    {
        constexpr Scalar pi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899863;
        return pi * r * r;
    }

    template<typename Scalar>
    Scalar intersecting_spheres_weight(const Scalar& R, const Scalar& r, const Scalar& d)
    {
        constexpr Scalar an_arbitrary_slop_factor = std::numeric_limits<Scalar>::epsilon() * 5;
        const Scalar _r = std::min(r, d);
        if (d < an_arbitrary_slop_factor) return std::numeric_limits<Scalar>::max(); // distance is nearly 0
        else if ((R + _r) <= d) return (Scalar)0; // the circles are non-intersecting
        else return circle_circle_intersection_area(R, _r, d) / circle_area(_r);
    }

    template < typename Scalar
             , typename Src
             , typename Demo
             , typename DemoList
             >
    Scalar intersecting_spheres_radius(const Src& q, const Demo& demo1, const DemoList& demo)
    {
        return std::transform_reduce(std::begin(demo), std::end(demo), std::numeric_limits<Scalar>::max(),
                [&](Scalar r, Scalar radius) { return std::min(r, radius); },
                [&](const auto& demo2)
                {
                    if (id(demo1) == id(demo2)) return std::numeric_limits<Scalar>::max();
                    return norm(source(demo1) - source(demo2));
                });
    }

    template < typename Scalar
             , typename Src
             , typename DemoList
             , typename Dst
             >
    void intersecting_spheres_lite_query(const Src& q, const DemoList& demo, Dst& out)
    {
        Scalar r_q = std::transform_reduce(std::begin(demo), std::end(demo), std::numeric_limits<Scalar>::max()
                , [](const auto a, const auto b){return std::min(a, b);}
                , [&](const auto& d){return norm(source(d) - q);}
                );

        Scalar sum_of_weights = 0;

        for (const auto& d : demo)
        {
            Scalar radius = intersecting_spheres_radius<Scalar>(q, d, demo);
            Scalar distance = norm(source(d) - q);
            Scalar weight = intersecting_spheres_weight(r_q, radius, distance);
            const Dst& dest = destination(d);
            sum_of_weights += weight;
            Dst step = dest - out;
            Scalar scale = sum_of_weights <= (Scalar)0 ? (Scalar)0 : weight / sum_of_weights;
            Dst inc = scale * step;
            out += inc;
        }
    }

    template < typename Scalar
             , typename Vector
             , typename DemoList
             , typename RadiusList
             >
    void intersecting_spheres_update(const Vector& q, const DemoList& demo,
            RadiusList& radius)
    {
        std::transform(std::begin(demo), std::end(demo), std::begin(radius),
                [&](const auto& demo1)
        {
            return intersecting_spheres_radius<Scalar>(q, demo1, demo);
        });
    }

    template < typename Scalar
             , typename Vector
             , typename DemoList
             , typename DistanceList
             , typename RadiusList
             , typename WeightList
             >
    void intersecting_spheres_query(
            const Vector& q, const DemoList& demo, const RadiusList& radius, 
            Scalar& r_q, DistanceList& distance, WeightList& weights)
    {
        using std::begin;
        using std::end;
        std::transform(begin(demo), end(demo), begin(distance), Utility::distance_to(q));
        r_q = std::reduce(begin(distance), end(distance), std::numeric_limits<Scalar>::max(),
                [](Scalar a, Scalar b) {return std::min(a, b); });
        std::transform(std::begin(distance), std::end(distance), std::begin(radius), std::begin(weights),
                [&](Scalar d_n, Scalar r_n)
        {
            return intersecting_spheres_weight(r_q, r_n, d_n);
        });
    }
}
