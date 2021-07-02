# Lampshade Interpolator

This is a novel algorithm I devised while immersed in the visualizations
produced by the demo application. I was specifically looking at the
intersecting n-spheres algorithm's contours. I noticed that points tended to
"cast a shadow", i.e. that the space "behind" them was generally dominated by
only that point. This seemed like a useful property compared to e.g. inverse
weighted distance (IWD) where the region behind a point is mixed with other
points past a certain distance. This idea of casting shadows lead to the
formulation presented now.

The weight of each demonstration is given by the relative amount of "light"
cast by the demonstration. This kind of operation is very similar to the
working principle of an IWD algorithm. However, the lampshade algorithm adds an
additional step; as well as casting its own light, each demonstration point
occludes the light cast by other demonstrations, casting shadows. 

It is easiest to consider the calculation of a single weight `w` related to the
demonstration `d` with coordinates `s` in source space. The base weight is
given by the inverse of the distance from `s` to the query point `q` raised
to some power, exactly as in a simple IWD algorithm.

$$
w_{base} = \frac{1}{||q - s||}
$$

The demonstration $d$ casts a ray of light towards $q$. Its brightness
decreases proportionate to the distance between them.  The base weight is then
reduced by a factor depending on how much $d$'s light it is occluded by the
shades centered on the other demonstrations. The shades are modelled as
hyperspheres, and the occlusion factor is given by a simplified function that
depends only on the distance from the center of the shade $s_n$ to the ray cast
from $s$ to $q$. To calculate the distance, first determine the orthogonal
projection $o$ of the line from $s$ to $s_n$ onto the line from $s$ to $q$:

$$
u_n = \frac{(q - s) \dot (s_n - s)}{||q - s||^2}
o = s + u_n * (q - s)
$$

The distance to the center of the shade is given by the distance from $s_n$ to
the orthogonal projection:

$$
d_n = ||s_n - o||
$$

If $u$ is less than zero than it is behind the demonstration. If $u$ is greater
than one, then it is behind the query point. In either case it does not cast a
shadow. Otherwise, when $0 < u < 1$ then the demonstration associated with
$s_n$ occludes the ray cast by $s$, and its base weight is reduced by
multiplying it with the occlusion factor $l$. The occlusion factor can be
determined in myriad ways. A very simple model is given by the following
equation; some variations are discussed shortly.

$$
l_n = min(1, d_n / r_n)
$$

Where $r_n$ is the radius of the shade associated with the $nth$ demonstration.
The final weight $w$ is given by multiplying the base weight with the occlusion
factor given by each other demonstration:

$$
w = w_{base} * \prod_{n = 0}^{n < N}(l_n)
$$

## Implementation

At this time, only a glsl implementation is given for this algorithm. The C++
part below only defines the parameters and metadata associated with the
algorithm, and no query function is defined yet.

```cpp
// @+'interpolators'
constexpr const char * const BasicLampshadeNames[2] =
        { "power"
        , "radius"
        };

template<typename Demonstration>
struct BasicLampshade
{
    USING_INTERPOLATOR_DEMO_TYPES;
    struct Meta { Scalar dot, dist, base, loss, w; };
    INTERPOLATOR_PARAMETER_STRUCT_START(BasicLampshadeNames, 2)
        INTERPOLATOR_PARAMETER_MIN(2, 0.1, 0.001);
        INTERPOLATOR_PARAMETER_MAX(2, 10, 1000);
        INTERPOLATOR_PARAM_ALIAS(power, 0);
        INTERPOLATOR_PARAM_ALIAS(r, 1);
    INTERPOLATOR_PARAMETER_STRUCT_END
    static constexpr const char * name = "Basic Lampshade";
    static constexpr const char * frag = "demo/shaders/basic_lampshade.frag";
};
// @/

// @#'demo/shaders/basic_lampshade.frag'
#define POWER 0
#define RDIUS 1

@{common shader interpolator variables}

@{shader functions}

void setup(in vec2 q) {}

float calculate_weight(in vec2 q, in int m)
{
    vec2 s = vec2(d.s[0], d.s[1]);
    vec2 diff = q - s;
    float dotd = dot(diff, diff);
    float dist = sqrt(dotd);
    float base = pow(dist, 4.0);//r[POWER]);
    float loss = 1.0;
    float u_n = -1.0;
    float d_n = r[RDIUS];
    for (int n = 0; n < N; ++n)
    {
        if (m == n) continue;
        load_demonstration(n);
        vec2 s_n = vec2(d.s[0], d.s[1]);
        vec2 snifs = s_n - s;
        float dots  = dot(  s,   s);
        float snots = dot(s_n,   s);
        float dotn  = dot(s_n, s_n);
        u_n = (dots + dotn - 2.0 * snots) / dot(snifs, diff); // this one works pretty intuitively, but the lens snaps on abruptly and discontinuously when points are close together. It would be nice if it eased in a bit

        if (u_n <= 0.0) continue;

        vec2 k = s + u_n * diff;
        d_n = distance(s_n, k);
        float l = min(1.0, d_n / r[RDIUS]);
        if (u_n > 1.0) l = max(l, min(1.0, distance(k, q) / (r[POWER]*10.0)));
        loss = loss * l;
    }
    return loss / base;
    //return vec4(float(0.0 < v_n && v_n < 1.0), float(0.0 < u_n), float(d_n < r[RDIUS]) * d.p[2], 1.0);//base * loss;
}

//void main()
//{
//    vec2 q = vec2(position.x * w/2.0, position.y * h/2.0);
//    load_demonstration(0);
//    colour = calculate_weight(q, 0);
//    //load_demonstration(1);
//    //float sm = weight + calculate_weight(q, 1);
//    //load_demonstration(0);
//    //weight = weight / sm;
//    //colour = vec4(weight * d.p[0], weight * d.p[1], weight * d.p[2], 1.0);
//}

@{shader main}
// @/
```
