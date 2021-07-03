# Inverse Distance Interpolator

The `[nodes]` object in Max/MSP, [originally created by Andrew
Benson](https://cycling74.com/forums/nodes), provides a simple and accessible
interface for 2-to-N-dimensional mappings. A detailed reference on its
implementation is not readily available, nor is the source code, but from [the
description of the object in the Max/MSP help
documentation](https://docs.cycling74.com/max7/refpages/nodes) and a cursory
examination of its runtime behaviour, it's not difficult to surmise how the
internals operate.

The algorithm can be considered as a kind of inverse weighted distance
interpolator with two distinctive modifications: the weight is linearly
proportional to the distance from the query to the node, and each node's
influence is limited to a certain radius. Although the object found in the
Max/MSP distribution is limited to a two dimensional input, there's actually
no technicaly reason for this limitation. Any input space that permits a
distance calculation would work equally as well.

Unlike most of the algorithms presented in this repository, the nodes algorithm
has the ability to output zero for all "weights"; this situation arises when
the query point is outside of the radius of all nodes. It is natural that the
algorithm should do this since it isn't meant exclusively for preset
interpolation. But in case that is its intended use, there are two obvious
alternatives for how to deal with the situation:

1) Just output all zeros if all the weights are zeros.

2) Allow the user to specify a background demonstration, and incorporate the
background into the interpolation if the sum of all weights is less than zero.

For our purposes, I've opted for the first choice in the GLSL implementation
and the second choice in the C++ implementation, primarily as a matter of
convenience; adding a background to the GLSL implementation would require
modifying the main shader routine or repeating large parts of it for the one
interpolator. Perhaps the GLSL implementations will eventually be more general
and better able to support this kind of interpolator-specific idiosyncrasy.
Until then, the background demo can be specified as a member variable of the
C++ implementation.

## Implementation

```cpp
// @+'interpolators'
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
// @/

// @#'demo/shaders/nodes.frag'
#define NSIZE 0

@{common shader interpolator variables}

@{shader functions}

void setup(in vec2 q) {}

float calculate_weight(in vec2 q, in int n)
{
    load_demonstration(n);
    vec2 s = vec2(d.s[0], d.s[1]);
    float dist = distance(q, s);
    return max(0.0, 1.0 - dist / r[NSIZE]);
}

@{shader main}
// @/
```
