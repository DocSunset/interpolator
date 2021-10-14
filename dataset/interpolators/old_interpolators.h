#ifndef INTERPOLATORS_H
#define INTERPOLATORS_H

#include<limits>
#include<cmath>
#include<iostream>
#include<cstddef>

#define USING_INTERPOLATOR_DEMO_TYPES \
using Demo = Demonstration; \
using Scalar = typename Demo::Scalar; \
using SVector = typename Demo::SVector; \
using PVector = typename Demo::PVector

#define USING_INTERPOLATOR_TYPES \
using Demo = typename Interpolator::Demo; \
using Scalar = typename Interpolator::Scalar; \
using SVector = typename Interpolator::SVector; \
using PVector = typename Interpolator::PVector; \
using Meta = typename Interpolator::Meta; \
using Para = typename Interpolator::Para
template <typename T, size_t N> constexpr size_t countof(T(&)[N]) { return N; }

#define INTERPOLATOR_PARAMETER_STRUCT_START(...) \
struct Para \
{ \
    const Scalar& operator[] (std::size_t n) const {return data[n];} \
          Scalar& operator[] (std::size_t n)       {return data[n];} \
    static constexpr const char * const names[] = {__VA_ARGS__};\
    static constexpr std::size_t size() {return countof(names);} \
    Scalar data[countof(names)];\

#define INTERPOLATOR_PARAMETER_MIN(...)\
    Scalar min[countof(names)] = {__VA_ARGS__}

#define INTERPOLATOR_PARAMETER_MAX(...)\
    Scalar max[countof(names)] = {__VA_ARGS__}

#define INTERPOLATOR_PARAM_ALIAS(alias, idx)\
const Scalar& alias() const {return data[idx];} \
      Scalar& alias()       {return data[idx];}

#define INTERPOLATOR_PARAMETER_STRUCT_END };

namespace Interpolators
{
    template<typename Scalar_T, typename ID, typename S_T, typename P_T>
    struct Demo
    {
        using Scalar = Scalar_T;
        using SVector = S_T;
        using PVector = P_T;

        SVector s;
        PVector p;
        ID id;
    };

    template<typename Scalar, std::size_t N>
    struct ParameterBase
    {
    };

    template<typename Scalar>
    struct ParameterBase<Scalar, 0>
    {
        const Scalar& operator[] (std::size_t n) const {return zero;}
              Scalar& operator[] (std::size_t n)       {return zero;}
        const char * name(std::size_t n) {return "";}
        static constexpr std::size_t size() {return 0;}
    private:
        Scalar zero{0};
    };

    template<typename Demonstration>
    struct IntersectingNSpheres
    {
        USING_INTERPOLATOR_DEMO_TYPES;
        struct Meta { Scalar r = 0, d = 0, w = 0; };
        using Para = ParameterBase<Scalar, 0>;

        bool dynamic_demos = true;
        mutable Scalar r_q;

        Scalar circle_circle_intersection_area(
                const Scalar& R, 
                const Scalar& r, 
                const Scalar& d) const
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

        Scalar circle_area(const Scalar& r) const
        {
            constexpr Scalar pi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899863;
            return pi * r * r;
        }

        Scalar intersecting_spheres_weight(const Scalar& R, const Scalar& r, const Scalar& d) const
        {
            return circle_circle_intersection_area(R, r, d) / circle_area(r);
        }

        template<typename DemoList, typename MetaList, typename ParaList>
        PVector query(const SVector& q, const DemoList& demo, const ParaList& para,
                MetaList& meta, PVector& weighted_sum) const
        {
            Scalar sum_of_weights = 0;
            if (demo.size() < 1) return weighted_sum;
            if (demo.size() != meta.size()) return weighted_sum;

            // step 1
            r_q = std::numeric_limits<Scalar>::max();
            for (std::size_t i = 0; i < demo.size(); ++i)
            {
                const SVector& s_n = demo[i].s;
                const PVector& p_n = demo[i].p;
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

            for (std::size_t i = 0; i < demo.size(); ++i)
            {
                const SVector& s_n = demo[i].s;
                const PVector& p_n = demo[i].p;
                Scalar& r_n = meta[i].r;
                Scalar& d_n = meta[i].d;
                Scalar& w_n = meta[i].w;

                if (dynamic_demos)
                {
                    // step 2
                    r_n = std::numeric_limits<Scalar>::max();
                    for (const Demo& demo2 : demo)
                    {
                        if (demo[i].id == demo2.id) continue; // demo cannot be its own nearest neighbour
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
                w_n = intersecting_spheres_weight(r_q, r_n < d_n ? r_n : d_n, d_n);

                // step 4
                weighted_sum = weighted_sum + w_n * p_n;
                sum_of_weights = sum_of_weights + w_n;
            }

            // step 5
            for (Meta& m : meta) { m.w = m.w / sum_of_weights; }
            weighted_sum = (1 / sum_of_weights) * weighted_sum;

            return weighted_sum;
        }

        static constexpr const char * name = "Intersecting N-Spheres";
        static constexpr const char * frag = "demo/shaders/intersecting_n-spheres.frag";
    };
    // after e.g. Todoroff 2009 ICMC
    template<typename Demonstration>
    struct InverseDistance
    {
        USING_INTERPOLATOR_DEMO_TYPES;
        struct Meta { Scalar d = 0, w = 0; };
        INTERPOLATOR_PARAMETER_STRUCT_START( "power"
                                           , "minimum_distance"
                                           , "minimum_radius"
                                           , "mass"
                                           )
            INTERPOLATOR_PARAMETER_MIN(0.1, -1000, -1000, 0.1);
            INTERPOLATOR_PARAMETER_MAX( 3,  1000,  1000,  10);
            INTERPOLATOR_PARAM_ALIAS(power, 0);
            INTERPOLATOR_PARAM_ALIAS(d_min, 1);
            INTERPOLATOR_PARAM_ALIAS(r_min, 2);
            INTERPOLATOR_PARAM_ALIAS(r, 3);
        INTERPOLATOR_PARAMETER_STRUCT_END
        
        Scalar sphere_of_influence = -1;

        template<typename DemoList, typename MetaList, typename ParaList>
        PVector query(const SVector& q, const DemoList& demo, const ParaList& para,
                MetaList& meta, PVector& weighted_sum)
        {
            Scalar sum_of_weights = 0;
            std::size_t i, N = demo.size();
            if (N < 1) return weighted_sum;
            if (N != meta.size()) return weighted_sum;

            for (i=0; i<N; ++i)  { meta[i].d = (demo[i].s - q).norm(); }
            for (i=0; i<N; ++i)  
            { 
                auto powr = pow( meta[i].d - para[i].r_min(), para[i].power()*para[i].power());
                meta[i].w = para[i].r() / std::max(powr, para[i].d_min());
            }
            for (i=0; i<N; ++i)
            {
                if (   sphere_of_influence < 0 
                   ||  meta[i].d < sphere_of_influence
                   )
                {
                    weighted_sum = weighted_sum + meta[i].w * demo[i].p;
                    sum_of_weights = sum_of_weights + meta[i].w;
                }
                // else don't add meta[i] to the mix
            }

            for (Meta& m : meta) { m.w = m.w / sum_of_weights; }

            return weighted_sum = (1 / sum_of_weights) * weighted_sum;
        }

        static constexpr const char * name = "Inverse Weighted Distance";
        static constexpr const char * frag = "demo/shaders/inverse_distance.frag";
    };
    template<typename Demonstration>
    struct BasicLampshade
    {
        USING_INTERPOLATOR_DEMO_TYPES;
        struct Meta { Scalar dot, dist, base, loss, w; };
        INTERPOLATOR_PARAMETER_STRUCT_START( "dropoff power"
                                           , "brightness"
                                           , "lens radius"
                                           , "lens thickness"
                                           )
            INTERPOLATOR_PARAMETER_MIN(0.1, 0.1, 0.001, 0.001);
            INTERPOLATOR_PARAMETER_MAX( 10,  10,  1000,  1000);
            INTERPOLATOR_PARAM_ALIAS(power, 0);
            INTERPOLATOR_PARAM_ALIAS(brightness, 1);
            INTERPOLATOR_PARAM_ALIAS(radius, 2);
            INTERPOLATOR_PARAM_ALIAS(thickness, 3);
        INTERPOLATOR_PARAMETER_STRUCT_END
        static constexpr const char * name = "Basic Lampshade";
        static constexpr const char * frag = "demo/shaders/basic_lampshade.frag";
    };
    // after e.g. Todoroff 2009 ICMC
    template<typename Demonstration>
    struct Nodes
    {
        USING_INTERPOLATOR_DEMO_TYPES;
        struct Meta { Scalar d, w = 0; };
        INTERPOLATOR_PARAMETER_STRUCT_START("nsize")
            INTERPOLATOR_PARAMETER_MIN(0);
            INTERPOLATOR_PARAMETER_MAX(1000);
            INTERPOLATOR_PARAM_ALIAS(nsize, 0);
        INTERPOLATOR_PARAMETER_STRUCT_END

        Demonstration background{};

        template<typename DemoList, typename MetaList, typename ParaList>
        PVector query(const SVector& q, const DemoList& demo, const ParaList& para,
                MetaList& meta, PVector& weighted_sum)
        {
            Scalar sum_of_weights = 0;
            std::size_t i, N = demo.size();
            if (N < 1) return weighted_sum;
            if (N != meta.size()) return weighted_sum;

            Scalar max_weight = 0;
            std::size_t weights;
            for (i=0; i<N; ++i)  { meta[i].d = (demo[i].s - q).norm(); }
            for (i=0; i<N; ++i)  
            {
                meta[i].w = std::max(0, 1 - meta[i].d / para[i].nsize());
                if (meta[i].w > 0)
                {
                    ++weights;
                    max_weight = meta[i].w;
                }
            }
            if (weights > 1)
            {
                for (i=0; i<N; ++i)  { weighted_sum = weighted_sum + meta[i].w * demo[i].p; }
                for (Meta& m : meta) { sum_of_weights = sum_of_weights + m.w; }
                for (Meta& m : meta) { m.w = m.w / sum_of_weights; }
                return weighted_sum = (1 / sum_of_weights) * weighted_sum;
            }
            else if (weights == 0) return background.p;
            else // (weights == 1)
            {
                for (Meta& m : meta) if (m.w > 0)
                    return weighted_sum + ( (1-max_weight) * background.p );
            }
        }

        static constexpr const char * name = "Nodes";
        static constexpr const char * frag = "demo/shaders/nodes.frag";
    };
};
#endif
