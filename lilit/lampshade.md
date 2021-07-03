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
given by the inverse of the distance from `s` to the query point `q` raised to
some power, exactly as in a simple IWD algorithm.  The demonstration $d$ casts
a ray of light towards $q$. Its brightness decreases proportionate to the
distance between them.  

$$
w_{base} = \frac{1}{||q - s||}
$$

The base weight is then reduced by a factor depending on how much $d$'s light
is occluded by the shades centered on the other demonstrations. A variety of
approaches could be adopted to model the way the shades reduce the brightness
of the ray from $s$ to $q$. In my experiments I have arbitrarily opted to
imagine a translucent lens located at the occluding demonstration's coordinate
in source space $s_n$.  The lens has a radius $r_n$ and a thickness $h_n$, and
the lens is always oriented so that it is orthogonal to $q$ with its front face
closer to $q$ and its back face centered on $s_n$. It is completely opaque at
its center, and linearly transitions to completely transparent at its outer
edge.

The loss factor imposed by the lens at $s_n$ on the base weight for $d$ is
given by these equations:

$$
u_n = \frac{||s||^2 + ||s_n||^2 - 2(s_n \dot s)}{(s_n - s) \dot (q - s)}
k = s + u_n * (q - s)
d_n = ||s_n - k||
l_rn = min(1, d_n / r_n)
$$

Where $u_n$ is the factor that scales the direction vector $(q - s)$ so that it
intersects the plane defined by the back face of the lens at points $k$. In
case $u_n$ is negative, this implies that the lens is behind $s$ and therefore
has no influence on the ray from $s$ to $q$. If $0 < u_n < 1$, the back plane
is in between $q$ and $s$, and the distance to the center of the lense is given
by $d_n$. The loss factor in this case is based on the ratio $d_n / r_n$ with
a minimum 1 boundary condition specified, such that the loss factor varies from
0 when $d_n = 0$ (i.e. the center of the lens is directly in the path of the
ray and completely occluded), to 1 when $d_n = r$ (i.e. the ray intersects the
lens at its edge.

If $u_n$ is greater than one, this implies that the back plane is behind $q$.
In this case the loss factor is adjusted empirically as follows:

$$
l = min(1, max(l, ||q - k|| / h_n))
$$

Where the ratio $||q - k|| / h_n$ roughly models the lens having some amount
of thickness so that even when its back plane is behind $q$ it may still
partially occlude the ray if $q$ is inside the lens. This "thickness" adjustment
can also be omitted, but leads to the interpolator having discontinuous changes
in its topology at certain boundaries where the lens rotates so that $q$ is
suddenly occluded by an opaque part of the lens.

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
// @/

// @#'demo/shaders/basic_lampshade.frag'
#define POWER 0
#define BRIGHTNESS 1
#define RADIUS 2
#define THICKNESS 3

@{common shader interpolator variables}

@{shader functions}

void setup(in vec2 q) {}

float calculate_weight(in vec2 q, in int m)
{
    vec2 s = vec2(d.s[0], d.s[1]);
    vec2 diff = q - s;
    float dotd = dot(diff, diff);
    float dist = sqrt(dotd);
    float base = pow(dist, r[POWER] * r[POWER]);
    float loss = r[BRIGHTNESS];
    float u_n = -1.0;
    float d_n = r[RADIUS];
    for (int n = 0; n < N; ++n)
    {
        if (m == n) continue;
        load_demonstration(n);
        vec2 s_n = vec2(d.s[0], d.s[1]);
        vec2 snifs = s_n - s;
        float dots  = dot(  s,   s);
        float snots = dot(s_n,   s);
        float dotn  = dot(s_n, s_n);
        u_n = (dots + dotn - 2.0 * snots) / dot(snifs, diff);

        if (u_n <= 0.0) continue;

        vec2 k = s + u_n * diff;
        d_n = distance(s_n, k);
        float l = min(1.0, d_n / r[RADIUS]);
        if (u_n > 1.0) l = max(l, min(1.0, distance(k, q) / r[THICKNESS]));
        loss = loss * l;
    }
    return loss / base;
}

@{shader main}
// @/
```
