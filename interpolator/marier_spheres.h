#ifndef MARIER_SPHERES_INTERPOLATOR_H
#define MARIER_SPHERES_INTERPOLATOR_H
#include<limits>
#include<cmath>
#include<iostream>

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
Scalar marier_spheres_weight(const Scalar& R, const Scalar& r, const Scalar& d)
{
    return circle_circle_intersection_area(R, r, d) / circle_area(r);
}

template<typename Scalar, typename ID, typename SVector, typename PVector>
class MarierSpheresInterpolator
{
public:
    struct Demo { ID id; SVector s; PVector p; Scalar r, d, w;};
    Scalar q_radius;

    template<typename DemoList>
    PVector query(const SVector& q, DemoList& demos)
    {
        Scalar sum_of_weights = 0;
        PVector weighted_sum = PVector() * 0;
        if (demos.size() < 1) return weighted_sum;

        q_radius = std::numeric_limits<Scalar>::max();
        for (const auto& demo : demos)
        {
            Scalar d = (demo.s - q).norm();
            if (d < q_radius) q_radius = d;
        }

        for (auto& demo : demos)
        {
            demo.d = (demo.s - q).norm();
            constexpr Scalar an_arbitrary_slop_factor = 
                std::numeric_limits<Scalar>::epsilon() * 5;
            if (demo.d <= an_arbitrary_slop_factor) return demo.p;
        
            demo.r = std::numeric_limits<Scalar>::max();
            for (const auto& demo2 : demos)
            {
                if (demo.id == demo2.id) continue;
                Scalar d = (demo.s - demo2.s).norm();
                if (d < demo.r) demo.r = d;
            }
            if (demo.d < demo.r) demo.r = demo.d;
        
            if ((q_radius + demo.r) < demo.d) 
            {
                demo.w = 0;
                continue; // the circles are non-intersecting
            }
            demo.w = marier_spheres_weight(q_radius, demo.r, demo.d);
            sum_of_weights = sum_of_weights + demo.w;
            weighted_sum = weighted_sum + demo.w * demo.p;
        }
        weighted_sum = (1 / sum_of_weights) * weighted_sum;
        for (auto& demo : demos) { demo.w = demo.w / sum_of_weights; }
        return weighted_sum;
    }
};
#endif