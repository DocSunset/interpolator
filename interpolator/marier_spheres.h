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
        PVector query(const SVector& q, const DemoList& demos,
                Metalist& meta, PVector& weighted_sum, Scalar& r_q,
                bool dynamic_demos = true)
        {
            Scalar sum_of_weights = 0;
            if (demos.size() < 1) return weighted_sum;
            if (demos.size() != meta.size()) return weighted_sum;

            // step 1
            r_q = std::numeric_limits<Scalar>::max();
            for (size_t i = 0; i < demos.size(); ++i)
            {
                const SVector& s_n = demos[i].s;
                const PVector& p_n = demos[i].p;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                d_n = (s_n - q).norm();
                if (d_n < r_q) r_q = d_n;

                constexpr Scalar an_arbitrary_slop_factor =
                        std::numeric_limits<Scalar>::epsilon() * 5;

                if (d_n <= an_arbitrary_slop_factor)
                {
                    w_n = 1;
                    weighted_sum = p_n;
                    return weighted_sum;
                }
            }

            for (size_t i = 0; i < demos.size(); ++i)
            {
                const SVector& s_n = demos[i].s;
                const PVector& p_n = demos[i].p;
                Scalar& r_n = meta[i].r;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                if (dynamic_demos)
                {
                    // step 2
                    r_n = std::numeric_limits<Scalar>::max();
                    for (const Demo& demo2 : demos)
                    {
                        if (demos[i].id == demo2.id) continue; // demo cannot be its own nearest neighbour
                        Scalar r = (s_n - demo2.s).norm();
                        if (r < r_n) r_n = r;
                    }
                }
            
                if ((r_q + r_n) < d_n)
                {
                    w_n = 0;
                    continue; // the circles are non-intersecting
                }

                // step 3
                w_n = marier_spheres_weight(r_q, r_n < d_n ? r_n : d_n, d_n);

                // step 4
                weighted_sum = weighted_sum + w_n * p_n;
                sum_of_weights = sum_of_weights + w_n;
            }

            // step 5
            for (Meta& m : meta) { m.w = m.w / sum_of_weights; }
            weighted_sum = (1 / sum_of_weights) * weighted_sum;

            return weighted_sum;
        }
    };
    template<int exponent = 1>
    class FastNonspheresInterpolator
    {
    public:
        struct Meta { Scalar d = 0, w = 0; };

        template<typename DemoList, typename Metalist>
        PVector query(const SVector& q, DemoList& demos,
                Metalist& meta, PVector& weighted_sum, Scalar& r_q,
                bool dynamic_demos = true)
        {
            Scalar sum_of_weights = 0;
            if (demos.size() < 1) return weighted_sum;

            r_q = std::numeric_limits<Scalar>::max();
            for (size_t i = 0; i < demos.size(); ++i)
            {
                const SVector& s_n = demos[i].s;
                const PVector& p_n = demos[i].p;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                d_n = (s_n - q).norm();
                if (d_n < r_q) r_q = d_n;

                constexpr Scalar an_arbitrary_slop_factor =
                        std::numeric_limits<Scalar>::epsilon() * 5;

                if (d_n <= an_arbitrary_slop_factor)
                {
                    w_n = 1;
                    weighted_sum = p_n;
                    return weighted_sum;
                }
            }

            for (size_t i = 0; i < demos.size(); ++i)
            {
                const PVector& p_n = demos[i].p;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                w_n = r_q / pow(d_n, exponent);

                weighted_sum = weighted_sum + w_n * p_n;
                sum_of_weights = sum_of_weights + w_n;
            }

            for (Meta& m : meta) { m.w = m.w / sum_of_weights; }
            weighted_sum = (1 / sum_of_weights) * weighted_sum;

            return weighted_sum;
        }
    };
    template<int exponent = 1>
    class InverseDistanceInterpolator1
    {
    public:
        struct Meta { Scalar d = 0, w = 0; };

        template<typename DemoList, typename Metalist>
        PVector query(const SVector& q, DemoList& demos,
                Metalist& meta, PVector& weighted_sum, Scalar& r_q,
                bool dynamic_demos = true)
        {
            Scalar sum_of_weights = 0;
            if (demos.size() < 1) return weighted_sum;

            for (size_t i = 0; i < demos.size(); ++i)
            {
                const SVector& s_n = demos[i].s;
                const PVector& p_n = demos[i].p;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                d_n = (s_n - q).norm();
                w_n = 1 / pow(d_n, exponent);

                weighted_sum = weighted_sum + w_n * p_n;
                sum_of_weights = sum_of_weights + w_n;
            }

            for (Meta& m : meta) { m.w = m.w / sum_of_weights; }
            weighted_sum = (1 / sum_of_weights) * weighted_sum;

            return weighted_sum;
        }
    };
    template<int exponent = 1>
    class InverseDistanceInterpolator2
    {
    public:
        struct Meta { Scalar d = 0, w = 0; };

        template<typename DemoList, typename Metalist>
        PVector query(const SVector& q, DemoList& demos,
                Metalist& meta, PVector& weighted_sum, Scalar& r_q,
                bool dynamic_demos = true)
        {
            Scalar sum_of_weights = 0;
            if (demos.size() < 1) return weighted_sum;

            for (size_t i = 0; i < demos.size(); ++i)
            {
                const SVector& s_n = demos[i].s;
                const PVector& p_n = demos[i].p;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                d_n = (s_n - q).norm();
            }

            for (size_t i = 0; i < demos.size(); ++i)
            {
                const SVector& s_n = demos[i].s;
                const PVector& p_n = demos[i].p;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                w_n = 1 / pow(d_n, exponent);

                weighted_sum = weighted_sum + w_n * p_n;
                sum_of_weights = sum_of_weights + w_n;
            }

            for (Meta& m : meta) { m.w = m.w / sum_of_weights; }
            weighted_sum = (1 / sum_of_weights) * weighted_sum;

            return weighted_sum;
        }
    };
    template<int exponent = 1>
    class InverseDistanceInterpolator3
    {
    public:
        struct Meta { Scalar d = 0, w = 0; };

        template<typename DemoList, typename Metalist>
        PVector query(const SVector& q, DemoList& demos,
                Metalist& meta, PVector& weighted_sum, Scalar& r_q,
                bool dynamic_demos = true)
        {
            Scalar sum_of_weights = 0;
            if (demos.size() < 1) return weighted_sum;

            for (size_t i = 0; i < demos.size(); ++i)
            {
                const SVector& s_n = demos[i].s;
                const PVector& p_n = demos[i].p;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                d_n = (s_n - q).norm();
            }

            for (size_t i = 0; i < demos.size(); ++i)
            {
                const SVector& s_n = demos[i].s;
                const PVector& p_n = demos[i].p;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                w_n = 1 / pow(d_n, exponent);
            }

            for (size_t i = 0; i < demos.size(); ++i)
            {
                const SVector& s_n = demos[i].s;
                const PVector& p_n = demos[i].p;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                weighted_sum = weighted_sum + w_n * p_n;
            }

            for (size_t i = 0; i < demos.size(); ++i)
            {
                const SVector& s_n = demos[i].s;
                const PVector& p_n = demos[i].p;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                sum_of_weights = sum_of_weights + w_n;
            }

            for (Meta& m : meta) { m.w = m.w / sum_of_weights; }

            weighted_sum = (1 / sum_of_weights) * weighted_sum;

            return weighted_sum;
        }
    };
};
#endif
