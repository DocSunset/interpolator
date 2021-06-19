#ifndef MARIER_SPHERES_INTERPOLATOR_H
#define MARIER_SPHERES_INTERPOLATOR_H
#include<limits>
#include<cmath>
#include<iostream>
#include<cstddef>

using std::size_t;

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
struct Interpolators
{
    struct Demo { ID id; SVector s; PVector p; };
    class IntersectingNSpheresInterpolator
    {
    public:
        struct Meta { Scalar r = 0, d = 0, w = 0; };

        template<typename DemoList, typename Metalist>
        PVector query(const SVector& q, const DemoList& demo,
                Metalist& meta, PVector& weighted_sum, Scalar& r_q,
                bool dynamic_demos = true)
        {
            Scalar sum_of_weights = 0;
            size_t j, i, N = demo.size();
            if (N < 1) return weighted_sum;
            if (N != meta.size()) return weighted_sum;

            r_q = std::numeric_limits<Scalar>::max();

            for (i=0; i<N; ++i)  { meta[i].d = (demo[i].s - q).norm(); }

            constexpr Scalar an_arbitrary_slop_factor =
                    std::numeric_limits<Scalar>::epsilon() * 5;
            for (i=0; i<N; ++i) if (meta[i].d <= an_arbitrary_slop_factor)
                return weighted_sum = demo[i].p;

            for (Meta& m : meta) { r_q = std::min(r_q, m.d); }

            if (dynamic_demos) 
            {
                for (i = 0; i < N; ++i) 
                {
                    meta[i].r = std::numeric_limits<Scalar>::max();
                    for (j = 0; j < N; ++j)
                    {
                        if (demo[i].id == demo[j].id) continue; // demo cannot be its own nearest neighbour
                        meta[i].r = std::min(meta[i].r, (demo[i].s - demo[j].s).norm());
                    }
                }
            }

            for (i=0; i<N; ++i) 
            { 
                if ((r_q + meta[i].r) < meta[i].d) meta[i].w = 0;
                else 
                {
                    meta[i].w = marier_spheres_weight(r_q, std::min(meta[i].r, meta[i].d), meta[i].d); 
                    weighted_sum = weighted_sum + meta[i].w * demo[i].p; 
                    sum_of_weights = sum_of_weights + meta[i].w;
                }
            }


            return weighted_sum = (1 / sum_of_weights) * weighted_sum;
        }
    };
    template</*int d_min, int r_min,*/ int power = 1>
    class InverseDistanceInterpolator /* after e.g. Todoroff 2009 ICMC */
    {
    public:
        struct Meta { Scalar d = 0, w = 0; };

        template<typename DemoList, typename Metalist>
        PVector query(const SVector& q, DemoList& demo,
                Metalist& meta, PVector& weighted_sum, Scalar& r_q,
                bool dynamic_demos = true)
        {
            Scalar sum_of_weights = 0;
            size_t i, N = demo.size();
            if (N < 1) return weighted_sum;
            if (N != meta.size()) return weighted_sum;

            for (i=0; i<N; ++i)  { meta[i].d = (demo[i].s - q).norm(); }
            for (Meta& m : meta) { m.w = 1 / pow(m.d, power); }
            for (i=0; i<N; ++i)  { weighted_sum = weighted_sum + meta[i].w * demo[i].p; }
            for (Meta& m : meta) { sum_of_weights = sum_of_weights + m.w; }
            for (Meta& m : meta) { m.w = m.w / sum_of_weights; }

            return weighted_sum = (1 / sum_of_weights) * weighted_sum;
        }
    };
};
#endif
