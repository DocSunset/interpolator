# Intersecting N-Spheres Interpolation

## Description

Martin Marier introduced the *intersecting n-spheres interpolation* algorithm
as part of his work with The Sponge, a deformable digital musical instrument
which Marier has been developing for over a decade. This algorithm produces an
interpolated output that is smooth (i.e. continuously differentiable),
continuous, maps from any number of source dimensions to any number of
destination dimensions, exactly passes through the demonstrated destination
points, and locally depends only on nearby demonstrations. The algorithm also
imposes no constraints on the placement of source and destination presets,
unlike other algorithms which might require points to be evenly distributed or
quantized to a grid. Furthermore, the algorithm allows for the position of
presets to be dynamically varied in real-time, allowing the possibility for
higher-level mappings such as navigating destination presets for the preset
interpolation system itself using a higher-level preset interpolator. 

Intersecting n-spheres interpolation produces weights for a weighted sum
according to the following procedure. In summary, for each demonstration two
circles are drawn: one centered on the source vector of the demonstration, and
one centered on the query vector (again in source space). The radius of either
circle is given by the distance from the circle's center point (either the
query vector or the source vector in a demonstration) to its nearest neighbour
in source space (which may again be either the query point or a source vector
in a demonstration). The weight for each demonstration is given by the ratio of
the area of the circle centered on the demonstration's source vector, and the
area of the intersection of that circle with the circle centered on the query
point. In a practical implementation, the procedure in full proceeds as
follows.

Given a query point $q$, and a list of demonstrations consisting of $N$ points
in source space $s_n$ paired with the same number of points in destination
space $p_n$:

1. Determine the distance $d_n$ from each demonstrated source point $s_n$ to
the cursor point $q$. At the same time, search for the nearest neighbour of $q$
and record its distance from $q$ as $r_q$.

2. For every point demonstrated point in source space $s_n$, calculate and
record the distance $r_n$ from that point to its nearest neighbour in source
space (not including $q$).

3. The weight $w_n$ associated with the destination vector $p_n$ of the $n$th
demonstration is given by $A_x(r_q, min(r_n, r_q), d_n) / A(r_n)$ where $A$ is
the area of the circle centered on $s_n$ with radius $r_n$, and where $A_x$ is
the area of the intersection of two circles:

- The circle centered on $s_n$ with radius given by the minimum between $r_n$ 
  or $r_q$,
- and the circle centered on $q$ with radius $r_q$,
- with the circles defined to lay on the same plane, which may be any plane
  passing through both $s_n$ and $q$.

Note that $A_x$ depends only on the radii of the two circles $r_q$ and $r_n$,
and the distance $d_n$ between them.

4. Calculate the interpolated output $p$ in destination space as the weighted
sum of demonstrated outputs $p_n$.

5. Normalize the weights (or equivalently the weighted sum) so that their sum
is equal to one by dividing each weight $w_n$ by the mean of all the weights
$\mu = \frac{1}{N}\sum_{n = 0}^{N - 1} w_n$

$A_x$ is given by the following function:

$$
\begin{aligned}
A_x(R, r, d) &= r^2 cos^(-1) ( \frac{d^2 + r^2 - R^2}{2dr} ) \\
             &+ R^2 cos^(-1) ( \frac{d^2 + R^2 - r^2}{2dR} ) \\
             &- \frac{\sqrt{(-d + r + R)(d + r - R)(d - r + R)(d + r + R)}}{2}
\end{aligned}
$$

And $A$ is given by the usual equation for the area of a circle:
$$
A(r) = \pi r^2
$$

The algorithm, despite its name, actually doesn't involve any n-spheres.
Intead, 2D circles are used, simplifying the algorithm while producing results
that "are predictable and feel natural to the user" according to Marier.

## Implementation

The weighted sum in Marier's algorithm implies that the destination space is
some kind of vector space, while the requirement for a notion of distance
implies that the source space is a normed vector space, most often an Euclidean
vector space. These assumptions are represented in the implementation by the
assumption that the templated `PVector` for the destination space will have an
operator for multiplication by a scalar, and addition of two vectors, and that
the templated `SVector` for the source space will have the same two operators
as well as a method `norm` that returns the length of a vector.

The functions to compute the weight for a single destination vector are
generally straightforward to implement based on the equations above.

One necessary quirk: when the interpolator is used with `float` scalars, the
argument to the `acos` and `sqrt` calls sometimes fall just outside the domain
of the functions. Boundary conditions are checked if `Scalar` is `float`: in
testing, the `acos` args only ever fell at or slightly above `1.0f`, so this
is the only condition which is addressed by the boundary check for these args.

```cpp
// @='intersecting n-spheres weight functions'
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
// @/
```

And as GLSL shader functions:

```cpp
// @='glsl intersecting n-spheres weight functions'
float circle_circle_intersection_area(
        in float R,
        in float r,
        in float d)
{
    float d2 = d * d;
    float r2 = r * r;
    float R2 = R * R;
    float two_d = 2.0 * d;
    float arg1 = (d2 + r2 - R2)/(two_d * r);
    float arg2 = (d2 + R2 - r2)/(two_d * R);
    float arg3 = (-d+r+ R) * (d+r-R) * (d-r+R) * (d+r+R);
    float a, b, c;
    if (arg1 > 1.0) a = 0.0;
    else a = r2 * acos(arg1);
    if (arg2 > 1.0) b = 0.0;
    else b = R2 * acos(arg2);
    if (arg3 < 0.0) c = 0.0;
    else c = sqrt(arg3) / 2.0;
    return a + b - c;
}

float circle_area(in float r)
{
    return pi * r * r;
}

float intersecting_spheres_weight(in float R, in float r, in float d)
{
    return circle_circle_intersection_area(R, r, d) / circle_area(r);
}
// @/
```

Step 2 above is an instance of the "all nearest neighbours" problem. We are
given a set of points (the demonstrated points in the source space plus the
cursor point) and for each point $p$ we need to determine the nearest other
point in the space $p_{\text{nearest}}$, which will be used to determine the
radius of the circle associated with $p$.

Based on a cursory and incomplete review of the literature, efficient
algorithms appear to exist which solve the all nearest neighbours problem in
$O(n\log n)$ time, or even in $O(n)$ time if certain constraints are met.
Unfortunately, comprehending (let alone implementing) the linear time
algorithms for this problem is non-trivial, so we opt to use a simpler
algorithm until such time as it becomes clear that optimization is necessary.

If we assume that the demonstrations may change at any time, then updating any
of the points in source space including the cursor point always requires the
nearest neighbour problem to be solved anew, because the nearest neighbour of
the updated point and any point that it was previously the nearest neighbour of
it all have to be updated. Because the position of the cursor point is assumed
to change whenever the interpolator is queried, this means that the nearest
neighbour problem must be solved for every query. The trivial implementation
used here (as well as in Marier's original Supercollider implementation of the
algorithm) solves this problem with $O(n^2)$ time complexity using plain old
brute force; the distance from every point to every other point is calculated,
and the nearest neighbour is found as the point with the least calculated
distance value. In Marier's practice this $O(n^2)$ time complexity hasn't been
reported as an issue, presumably because the number of data points has always
been reasonably low.

However, if we assume that the demonstrations have not changed since the last
query, then it should be unnecessary to solve the all nearest neighbours
problem.  Instead, the weights can be generated in linear time, since only the
distance from the query point to each demonstration needs to be calculated. 

In either case, the query function is logically pure. The result depends only
on the demonstration and query points. The output consists of the weighted sum,
but also the weights, distances, and radii calculated along the way. These
latter values are useful for interactive applications such as the OpenGL demo
presented later, since they allow the inner workings of the algorithm to be
rendered to the user.

The interpolation function is implemented as a class for a few reasons. This
allows the metadata and parameter structures unique to this interpolator to be
accessible through the type system whenever you have an instance of this
interpolator. The Intersecting N-Spheres algorithm doesn't have any parameters,
but other algorithms will require these, so a `Para` structure is declared to
provide a consistent interface.  The class is also used as the interface to set
interpolator-specific global parameters, such as the flag `dynamic_demos` that
is used to decide whether to skip the nearest-neighbours problem.  Finally, the
class is used to store interpolator-specific global outputs, such as the `r_q`
radius calculated in the N-Spheres algorithm.

```cpp
// @+'interpolators'
template<typename Demonstration>
struct IntersectingNSpheres
{
    USING_INTERPOLATOR_DEMO_TYPES;
    struct Meta { Scalar r = 0, d = 0, w = 0; };
    using Para = ParameterBase<Scalar, 0>;

    bool dynamic_demos = true;
    mutable Scalar r_q;

    @{intersecting n-spheres weight functions}

    template<typename DemoList, typename MetaList, typename ParaList>
    PVector query(const SVector& q, const DemoList& demo, const ParaList& para,
            MetaList& meta, PVector& weighted_sum) const
    {
        Scalar sum_of_weights = 0;
        if (demo.size() < 1) return weighted_sum;
        if (demo.size() != meta.size()) return weighted_sum;

        // step 1
        @{calculate radius r_q and distances d_n}

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
                @{calculate the radius r_n}
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
// @/

// @#'demo/shaders/intersecting_n-spheres.frag'
@{common shader interpolator variables}

@{shader functions}

@{glsl intersecting n-spheres weight functions}

float r_q;

void setup(in vec2 q)
{
    float fMaxFloat = intBitsToFloat(2139095039);
    @{glsl calculate r_q}
}

float calculate_weight(in vec2 q, in int n)
{
    float fMaxFloat = intBitsToFloat(2139095039);
    load_demonstration(n);
    vec2 s = vec2(d.s[0], d.s[1]);
    vec3 p = vec3(d.p[0], d.p[1], d.p[2]);
    float d_n = distance(q, s);
    @{glsl calculate the radius r_n}
    if ((r_q + r_n) < d_n) return 0.0;
    return intersecting_spheres_weight(r_q, min(r_n, d_n), d_n);
}

@{shader main}
// @/
```

One quirk of the above CPU implementation is that the output (`weighted_sum`)
is passed by variable. This is necessary to avoid having to initialize
`weighted_sum` to zero. By leaving this responsibility to the caller, the
implementation of the interpolator is more generic, only imposing the need for
operators for addition and multiplication by a scalar, and `size()` member and
index operator for the lists. In other words, the interpolator doesn't know how
to initialize a `PVector`, which could be a raw float array, a complicated
linear algebra library vector, or anything else, as long as it has addition and
multiplication by a scalar. Since `PVector` is so generic, the implementation
of the interpolator can't make assumptions about how to construct or initialize
it.

### Implementation details

Although every demonstration will be visited in the main loop of the query
function, the radius of the query point has to be known in advance in order to
calculate the weight associated with each demonstration. While searching for
the query point's nearest neighbour, it is convenient to also cache the
distance from the query point to every demonstration.

```cpp
// @='calculate radius r_q and distances d_n'
r_q = std::numeric_limits<Scalar>::max();
for (std::size_t i = 0; i < demo.size(); ++i)
{
    const SVector& s_n = demo[i].s;
    const PVector& p_n = demo[i].p;
    Scalar& d_n = meta[i].d;
    Scalar& w_n = meta[i].w;

    d_n = (s_n - q).norm();
    if (d_n < r_q) r_q = d_n;

    @{avoid numerical imprecision issues}
}
// @/
```

There's no way to allocate an array to cache the distances on the GPU, since
the number of demonstrations isn't known at compile time and you can't allocate
an array with a variable length on the stack, so the GPU just has to
recalculate the distance.

```cpp
// @='glsl calculate r_q'
r_q = fMaxFloat;
for (int n = 0; n < N; ++n)
{
    load_demonstration(n);
    vec2 s = vec2(d.s[0], d.s[1]);
    vec3 p = vec3(d.p[0], d.p[1], d.p[2]);
    float d_n = distance(q, s);
    r_q = min(d_n, r_q);
}
// @/
```

As the query point approaches the position of a demonstrated point, the size of
both $A_x$ and $A$ get smaller and smaller, which would likely lead to
numerical instability when calculating the weight $A_x/A$. For this reason, an
arbitrary amount of sloppiness is introduced into the determination of whether
the query point exactly coincides with a demonstrated point, so that the ratio
$A_x/A$ will not be calculated if the query is too close to a demonstration.
Instead, if the query is close enough to the demonstration then the exact value
of the demonstration is returned and further calculation of the weighted sum is
short-circuited.  This condition is checked while calculating the distance from
the current source vector to the query point, which takes place just before
searching for the radius of the circle centered on the source vector. Although
this also short circuits the recalculation of all the metadata calculated
during a query, it is presumed that this condition will be approached gradually
and encountered rarely and briefly, and that the metadata will therefore be
close enough for practical purposes.

```cpp
// @='avoid numerical imprecision issues'
constexpr Scalar an_arbitrary_slop_factor =
        std::numeric_limits<Scalar>::epsilon() * 5;

if (d_n <= an_arbitrary_slop_factor)
{
    w_n = 1;
    weighted_sum = p_n;
    return weighted_sum;
}
// @/
```

Finding the radius of the circle centered on the demonstration `demo` is done
by brute force search, as mentioned above. The distance from `demo` to every
other demonstration is checked, and the least distance is recorded. Finally,
the distance from the demonstration to the query point is checked, and this is
used if it is less than the distance from the demonstration to the nearest
other demonstration.

```cpp 
// @='calculate the radius r_n'
r_n = std::numeric_limits<Scalar>::max();
for (const Demo& demo2 : demo)
{
    if (demo[i].id == demo2.id) continue; // demo cannot be its own nearest neighbour
    Scalar r = (s_n - demo2.s).norm();
    if (r < r_n) r_n = r;
}
// @/

// @='glsl calculate the radius r_n'
float r_n = fMaxFloat;
for (int m = 0; m < N; ++m)
{
    if (n == m) continue;
    load_demonstration(m);
    vec2 s_m = vec2(d.s[0], d.s[1]);
    float r = distance(s, s_m);
    r_n = min(r, r_n);
}
load_demonstration(n);
// @/
```
