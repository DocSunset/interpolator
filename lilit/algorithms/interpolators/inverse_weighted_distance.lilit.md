# Inverse Distance Interpolator

Another simple interpolation algorithm is presented here. This algorithm is one
of the earliest proposed in the literature, with roots in the early 1980s with
the SYTER system developed at GRM. The implementation below follows the
presentation by Todoroff and colleagues in their 2009 paper at the
International Computer Music Conference, "1-d, 2-d and 3-d interpolation tools
for max/msp/jitter." Despite the name of their paper, the algorithm naturally
extends to arbitrary demonstration spaces.

The most basic version of the algorithm is a simplified gravitational model.
The weight for demonstration $d$ with source point $s$, the weight is simply
the inverse of the squared distance from the query $q$ to $s$:

$$
w = \frac{r}{||q - d||^2}
$$

Where $r$ accounts for the relative mass of the demonstration. A useful
extension is to allow an arbitrary positive exponent $p$:

$$
w = \frac{r}{||q - d||^p}
$$

Todoroff and colleagues also introduce a global $d_{min}$ parameter that serves
to prevent numerical instability, and $r_{min}$ parameter that allows a region
around the demonstration to be dedicated to that demonstration.

$$
w = \frac{r}{max(d_{min}, (||q - d|| - r_{min})^p)}
$$

Todoroff and colleagues also describe several methods for incorporating a
background "void" which is interpolated into the mix as a stable field which
the demonstrations perturb. At this time I have not implemented this extension
to the basic model.

Finally, Gibson and colleagues introduced a simple but quite useful extension
where only demonstrations within a given radius are considered for
interpolation. This allows the user to control the amount of locality of the
algorithm, defining a sphere of influence outside of which distant
demonstrations do not affect the output of the interpolator. The
sphere-of-influence extension can be disabled by setting the sphere radius to a
negative number.

## Implementation

```cpp
// @+'interpolators'
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
// @/

// @#'demo/shaders/inverse_distance.frag'
#define POWER 0
#define D_MIN 1
#define R_MIN 2
#define RADIUS 3

float sphere_of_influence = -1.0;

@{common shader interpolator variables}

@{shader functions}

void setup(in vec2 q) {}

float calculate_weight(in vec2 q, in int n)
{
    load_demonstration(n);
    vec2 s = vec2(d.s[0], d.s[1]);
    float dist = distance(q, s);
    if (   sphere_of_influence < 0.0
        ||  dist < sphere_of_influence)
    {
        float powr = pow(dist - r[R_MIN], r[POWER] * r[POWER]);
        return r[RADIUS] / max(powr, 0.0000001);
    }
    else return 0.0;
}

@{shader main}
// @/
```
