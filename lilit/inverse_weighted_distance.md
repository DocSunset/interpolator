# Inverse Distance Interpolator

Another simple interpolation algorithm is presented here. This algorithm is one
of the earliest proposed in the literature, with roots in the early 1980s with
the SYTER system developed at GRM. The implementation below follows the presentation
by Todoroff and colleagues in their 2009 paper at the International Computer
Music Conference, "1-d, 2-d and 3-d interpolation tools for max/msp/jitter."
Despite the name of their paper, the algorithm naturally extends to arbitrary
source and destination spatial dimensions.

```cpp
// @+'interpolators'
// after e.g. Todoroff 2009 ICMC
constexpr const char * const InverseDistanceNames[4] =
        { "power"
        , "minimum_distance"
        , "minimum_radius"
        , "gravity"
        };

template<typename Demonstration>
struct InverseDistance
{
    USING_INTERPOLATOR_DEMO_TYPES;
    struct Meta { Scalar d = 0, w = 0; };
    INTERPOLATOR_PARAMETER_STRUCT_START(InverseDistanceNames, 4)
        INTERPOLATOR_PARAM_ALIAS(power, 0);
        INTERPOLATOR_PARAM_ALIAS(d_min, 1);
        INTERPOLATOR_PARAM_ALIAS(r_min, 2);
        INTERPOLATOR_PARAM_ALIAS(r, 3);
    INTERPOLATOR_PARAMETER_STRUCT_END

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
            auto base = std::max(meta[i].d - para[i].r_min(), para[i].d_min());
            meta[i].w = para[i].r() / pow( base, para[i].power());
        }
        for (i=0; i<N; ++i)  { weighted_sum = weighted_sum + meta[i].w * demo[i].p; }
        for (Meta& m : meta) { sum_of_weights = sum_of_weights + m.w; }
        for (Meta& m : meta) { m.w = m.w / sum_of_weights; }

        return weighted_sum = (1 / sum_of_weights) * weighted_sum;
    }

    static constexpr const char * name = "Inverse Weighted Distance";
    static constexpr const char * frag = "demo/shaders/inverse_distance.frag";
};
// @/

// @#'demo/shaders/inverse_distance.frag'
#define POWER 0
#define D_MIN 1
#define R_MIN 2
#define RDIUS 3

@{common shader interpolator variables}

@{shader functions}

void setup(in vec2 q) {}

float calculate_weight(in vec2 q, in int n)
{
    load_demonstration(n);
    vec2 s = vec2(d.s[0], d.s[1]);
    vec3 p = vec3(d.p[0], d.p[1], d.p[2]);
    float dist = distance(q, s);
    float base = max(dist - r[R_MIN], r[D_MIN]);
    return r[RDIUS] / pow(base, r[POWER]);
}

@{shader main}
// @/
```
