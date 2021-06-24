# Introduction

In sound design, digital musical instrument design, and intermedia art in
general, creators often deal with complex generative systems with numerous
control parameters: sound and video synthesizers and processors, simulations,
and artificially intelligent agents are a few examples. These systems are often
capable of producing a combinatorially immense variety of interesting behaviors
(sounds, images, etc.) based on the state of their parameters. Much of the
creative work of using and interacting with these systems comes down to
exploring these vast spaces of possibilities, perhaps noting points of interest
along the way. Eventually, entire compositions, installations, or musical
instruments can be devised by selecting the paths to navigate within these
spaces of possibilities.

A useful tool in the exploration and navigation of these spaces is preset
interpolation. After manually recording points of interest in parameter space
as presets, preset interpolation algorithms are used to automatically move
between these predetermined a-priori interesting places, filling in the space
between them with intermediate locations that are likely, by proximity, to also
be interesting. This allows the user of a preset interpolation algorithm to
move easily through the space of possibilities demarcated by the points of
interest they record as presets, facilitating the discovery of other
interesting points in space and of interesting paths through space.

What is especially useful about preset interpolation is that it allows the user
to reframe the navigation of the control parameter space in terms of another
separate space of control sources. For instance, the presets in parameter space
can be associated with locations on a two dimensional map, often significantly
reducing the dimensionality of navigation with an associated reduction in the
difficulty of navigation. Alternatively, the control sources from a complex
gestural interface can be used instead of a two dimensional map, allowing the
destination space to be navigated by moving through the space of gestures
afforded by the interface. These kinds of mappings are often essential in the
design of digital musical instruments, and can be equally rewarding in other
intermedia artistic practices.

# Theory

The basic operating principle of many preset interpolation algorithms,
including the one presented here, is for the user to specify pairs of
source-to-destination associations in the form of presets in these two spaces.
As a concrete example, the user might be exploring the space of sounds produced
by a synthesizer with possibly a very large number of parameters. They might
explore for a while by manually configuring the synthesizer, and note several
particularly interesting preset sound configurations. Then, to facilitate live
performance with the synthesizer, the user may wish to control it with a
graphics tablet. They can associate several locations on the tablet with
different presets found earlier, and then use the preset interpolation
algorithm to navigate between these presets. The tablet locations can also be
thought of as presets, but located in the source space (the tablet's control
dimensions) rather than the destination space (the sound synthesizer
parameters).

To remain unambiguous, for the rest of this exposition preset points in source
space will be refered to as source vectors while preset points in destination
space will be referred to as destination vectors. The word vector reflects the
assumption inherent in most preset interpolation algorithms that the source and
destination spaces are both vector spaces, meaning that presets can be
meaningfully scaled and added together, and that there is a meaningful notion
of distance between presets.  A pair made of a source vector $s$ and a
destination vector $p$ to which the sources in the vicinity of $s$ should be
associated will be called a demonstration, $d = \{s, p\}$. This reflects an
interaction metaphor in which the user is thought to provide demonstrations of
action-to-output relationships, i.e. "when I do $s$, the interpolator should
output $p$."  The symbol $p$ is chosen for destination vectors based on the
common situtation in which the destination space consists of the control
parameters of some media generator such as a sound synthesizer, and to avoid
collision with $d$ for a demonstration of a source-to-destination pair.

The basic principle of many interpolation algorithms is to interpolate between
$N$ destination vectors $p_n$ producing an output vector $y$ in destination
space via a weighted sum with weights given by $\boldsymbol{w}$:

$$
y(\boldsymbol{w}) = \sum_{n = 0}^{N - 1} w_n * p_n
$$

The role of the preset interpolator is then reduced to simply producing and
applying a list of weights, usually as a function of a query or cursor vector
$q$ in the source space, and list $D$ of $N$ demonstrations.

$$
\boldsymbol{w}(q, D) = {w_0, w_1, w_2, ..., w_{n}} 
$$
$$
D = \{s_0, p_0\}, \{s_1, p_1\}, \{s_2, p_2\}, ..., \{s_n, p_n\}
$$

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
// @='weight functions'
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
struct IntersectingNSpheres
{
    struct Meta { Scalar r = 0, d = 0, w = 0; };
    struct Para { /* none */ };

    bool dynamic_demos = true;
    mutable Scalar r_q;

    @{weight functions}

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

};
// @/
```

One quirk of the above implementation is that the output (`weighted_sum`) is
passed by variable. This is necessary to avoid having to initialize
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
by brute force search, as mentioned above. The distance from `demo` to ever
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
```

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
struct InverseDistance /* after e.g. Todoroff 2009 ICMC */
{
    struct Meta { Scalar d = 0, w = 0; };
    struct Para 
    { 
        Scalar power = 8
        ,      d_min = std::numeric_limits<Scalar>::min()
        ,      r_min = 0
        ,      r = 1
        ;
    };

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
            auto base = std::max(meta[i].d - para[i].r_min, para[i].d_min);
            meta[i].w = para[i].r / pow( base, para[i].power); 
        }
        for (i=0; i<N; ++i)  { weighted_sum = weighted_sum + meta[i].w * demo[i].p; }
        for (Meta& m : meta) { sum_of_weights = sum_of_weights + m.w; }
        for (Meta& m : meta) { m.w = m.w / sum_of_weights; }

        return weighted_sum = (1 / sum_of_weights) * weighted_sum;
    }
};
// @/
```

# Interpolators Summary and Library

The interpolators are all gathered inside of a shared `struct`. This is done in
order to propagate the template parameters for the `Scalar, ID, SVector,` and
`PVector` types to all the interpolators so that in case the calling program
wants to use the same types for all interpolators they only need to declare
them once. The demonstration type, `struct Demo`, is also declared at this
level.

```cpp 
// @#'interpolator/marier_spheres.h'
#ifndef MARIER_SPHERES_INTERPOLATOR_H
#define MARIER_SPHERES_INTERPOLATOR_H
#include<limits>
#include<cmath>
#include<iostream>
#include<cstddef>


template<typename Scalar, typename ID, typename SVector, typename PVector>
struct Interpolators
{
    struct Demo { ID id; SVector s; PVector p; };
    @{interpolators}
};
#endif
// @/
```

# Demonstration

An demo program is provided in the repository that is helpful in exploring the
subtleties of the different algorithms presented here. The program randomly
generates a handful of source-destination demonstrations, and draw an image by
systematically querying the source space. As well as producing images that
might offer some intuition about the topology of the interpolated output, this
will also serve as a reasonable benchmark for the efficiency of the
implementation and compilation environment, if the number of pixels in the
image is large enough.  

For example, the initial implementation of the Intersecting N-Spheres
interpolator, when compiled with optimizations enabled and run on a circa 2012
ultrabook processor, could process about 15000 queries per second with 5
demonstrations, about 5000 per second with 15 demonstrations, 2200 with 25
demonstrations, and 1200 with 35 demonstrations.  As per the quadratic time
complexity of this implementation, the number of queries processed per second
decreases by a multiplicative factor as the number of demonstrations increases
linearly.  Performance on larger datasets is improved significantly, as
expected, if the quadratic-time all nearest neighbours problem is avoided by
assuming that demonstrations are unchanged between interpolations. Performance
may have changed significantly since that initial benchmark, due to a
combination of changes in the overall runtime (the original benchmark didn't
use SDL or the GPU for instance) and the implementation itself. But the numbers
given provide some indication of the relative performance of different
interpolator algorithms.

The demo program requires SDL2, and is designed to be compiled as a native or
WASM-based web application. 

## Usage

This section still needs to be written.

## Implementation

The starring roles are played by the interpolation algorithms, declared in a
tuple alongside the lists of metadata and parameters. The source space is
normalized 2D screen coordinates. The destination space is color.  The colour
interpolations are performed in [J_zA_zB_z colour
space](https://doi.org/10.1364/OE.25.015131), a colour representation that is
designed to provide the best balance of perceptual uniformity and iso-hue
linearity.  This is meant to ensure that the image produced reflects the
topology of the interpolation rather than the non-linearity of sRGB colour
space. The details of the conversion from RGB (assumed sRGB) through CIE XYZ to
J_zA_zB_z colour space and back are given below after the presentation of the
demo program.

```cpp 
// @='types'
using Scalar = float;
using ID = unsigned int;
using Vec2 = Eigen::Vector2f;
using RGBVec = Eigen::Vector3f;
using RGBAVec = Eigen::Vector4f;
using CIEXYZVec = Eigen::Vector3f;
using JzAzBzVec = Eigen::Vector3f;
using Texture = Eigen::Matrix<RGBAVec, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using Interpolator = Interpolators<Scalar, ID, Vec2, JzAzBzVec>;
// @/

// @='declare interpolators'
#define INTERPOLATOR(type, ...) std::make_tuple(type{}, std::vector<type::Meta>{}, std::vector<type::Para>{}, type::Para{__VA_ARGS__})
auto interpolators = std::make_tuple
        ( INTERPOLATOR(Interpolator::IntersectingNSpheres)
        , INTERPOLATOR(Interpolator::InverseDistance, 4, 0.001, 0.0)
        );
// @/
```

An overview of the program shows a typical event-loop driven SDL application.
In order to accomodate both Emscripten/web and native SDL build targets, the
loop is placed in a seperate function `void loop()` that can be called by the
browser's event loop or the main function loop depending on the platform.

```cpp
// @#'examples/interpolators_demo.cpp'
@{includes}
@{types}
@{declare interpolators}
@{colour conversions}

@{shaders}

struct Context
{
    @{globally visible state}
} context;

@{helper functions}

void loop ()
{
    @{poll event queue and handle events}

    if (context.redraw)
    {
        @{draw and refresh screen}
//        for(int row=0; row<context.texture.rows(); row++)
//            for(int col=0; col<context.texture.cols(); col++)
//                context.texture(row,col) = RGBAVec(row/((float)context.texture.rows()), col/((float)context.texture.cols()), 0, 1);

        write_gl_texture(context.texture, context.texture_gl);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, context.screen_quad.size());
    SDL_GL_SwapWindow(context.window);
}

int main()
{
    @{setup}

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, -1, 1);
#else
    while (not context.quit)
    {
        loop();
        SDL_Delay(33);
    }
#endif

    return EXIT_SUCCESS;
}
// @/
```

### Drawing

The drawing routine is deeply embedded in several layers of book keeping:
finding the active interpolator within the tuple, timing the drawing routine,
and finally the drawing routine itself.

The main loop of the program clears the screen, draws, and then presents the
drawing. Only one interpolator is drawn at a time (the active interpolator).

```cpp
// @+'globally visible state'
unsigned int active_interpolator = 0;
// @/
```

In order to find the `active_interpolator`, it is necessary to iterate over the
tuple of interpolators using a C++17 fold expression in a generic lambda that
calls the draw function and is applied over the whole tuple; this is the best
way I could think of to iterate over the tuple, and is necessary as a
consequence of the decision to use a tuple and avoid runtime polymorphism.
This is by no means the only way to achieve this (there are probably better
ways), but it's convenient not to have to write a base class for interpolators
at the stage of development I'm currently at. 

The iteration happens in the main loop. An index `i` initialized to 0. Each
call to the draw function is passed this index, and one interpolator. If the
index doesn't match the `active_interpolator`, then the function returns
without doing anything. In this way only the active interpolator is drawn.

```cpp
// @='draw and refresh screen'
unsigned int i = 0;
std::apply([&](auto& ... tuples) {((draw(i, tuples)), ...);}, interpolators);
context.redraw = false;
// @/

// @+'helper functions'
template<typename T>
void draw(unsigned int& i, T& tup)
{
    if (i++ != context.active_interpolator) return;

    auto& interpolator = std::get<0>(tup);
    auto& meta = std::get<1>(tup);
    auto& para = std::get<2>(tup);

    @{run the timer and draw}
}
// @/
```

The drawing routine itself is couched in another layer, which simply runs a
timer while the drawing takes place to provide a simple performance benchmark.

```cpp
// @='run the timer and draw'
@{a flag to keep track of static demos optimization}
auto start = std::chrono::high_resolution_clock::now();

for (unsigned int col = 0; col < context.texture.cols(); ++col)
{
    for (unsigned int row = 0; row < context.texture.rows(); ++row)
    {
        @{draw one pixel}
    }
}

auto stop = std::chrono::high_resolution_clock::now();
auto usec = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
std::cout << i-1 << ": Generated " << context.texture.cols() * context.texture.rows() << " interpolations in " << usec << " microseconds\n" 
        << "About " << 1000000 * context.texture.cols() * context.texture.rows() / usec << " interpolations per second" 
        << std::endl;
// @/
```

Finally, the drawing routine itself:

```cpp
// @='draw one pixel'
RGBVec out = {0, 0, 0};
auto q = Vec2{col/(Scalar)context.texture.cols(), row/(Scalar)context.texture.rows()};

JzAzBzVec interpolated_jab{0, 0, 0};
interpolator.query(q, context.demo, para, meta, interpolated_jab);

if (context.C) 
{
    @{draw contour lines}
}
else out = JzAzBz_to_RGB(interpolated_jab);

context.texture(row, col) = RGBAVec{out.x(), out.y(), out.z(), 1};
// @/
```

The default drawing mode simply draws the output of the interpolator (converted
to RGB). An alternative drawing mode shows contour lines that aim to directly
illustrate the topology of the weights used to interpolate between presets.
This kind of visualization is akin to a terrain map where instead of
illustrating elevation of the terrain, the contours illustrate the weight of a
preset. For guidance in reading the contour map, consider the following:

- lines of the same color refer to the weight of the same preset
- points on the sharp edge of a certain contour line all have the same weight
- the change in weight from one line to the next is always the same, given by
  `1/C` where `C` is the number of lines requested by the command line argument 
  (10 by default)
- lines spaced close together represent a rapid change in weight
- lines spaced far apart represent a gradual change
- when a preset maxes out so that no other presets are involved in the output,
  a grid of dots is drawn on top of the contour peak
- it's usually best to focus on contour lines of one colour at a time

Contour lines are drawn if the number of contour lines requested in the context
structure is greater than zero. Additionally, if there is a demonstration
selected (`context.grabbed` is not `nullptr`), contour lines are not drawn for
other demonstrations; note that in such cases the for loop over demonstrations
is broken after one pass, which itself doesn't even refer to the iteration
index.

The brightness calculation for contour lines is empirically set to look good
and aid clear intuition; there is no special reason the lines are drawn in
this particular way.

```cpp
// @+'globally visible state'
unsigned int C = 0;
// @/

// @='draw contour lines'
for (unsigned int n = 0; n < context.N; ++n)
{
    RGBVec rgb;
    Scalar w;
    if (context.grabbed) 
    {
        rgb = JzAzBz_to_RGB(context.grabbed->p);
        w = meta[context.grabbed_idx].w;
    }
    else 
    {
        rgb = JzAzBz_to_RGB(context.demo[n].p); 
        w = meta[n].w;
    }
    if (w >= 1.0 - std::numeric_limits<Scalar>::min() * 5)
    {
        // visualize maximum elevation with inverted colour dots
        out = (col % 3) + (row % 3) == 0 ? RGBVec{1,1,1} - rgb : rgb;
    }
    else
    {
        Scalar brightness = std::pow(std::fmod(w * context.C, 1.0f), 8);
        brightness = brightness * w;
        out += rgb * brightness;
    }
    if (context.grabbed) break;
}
// @/
```

```cpp
// @='shaders'
struct TextureQuad
{
    static constexpr const char * name = "texture quad";
    static constexpr const char * vert_name = "texture quad vertex shader";
    static constexpr const char * vert =
    "\
        #version 300 es\n\
        in vec2 pos;\n\
        out vec2 tex_coord;\n\
        const vec4 white = vec4(1.0);\n\
        \n\
        void main()\n\
        {\n\
            gl_Position = vec4(pos, 0.0, 1.0);\n\
            tex_coord   = vec2(pos[0] * 0.5 + 0.5, pos[1] * 0.5 + 0.5);\n\
        }\n\
    ";
    
    static constexpr const char * frag_name = "texture quad fragment shader";
    static constexpr const char * frag =
    "\
        #version 300 es\n\
        #ifdef GL_ES\n\
        precision highp float;\n\
        #endif\n\
        in vec2 tex_coord;\n\
        out vec4 fragColour;\n\
        uniform sampler2D tex_sampler;\n\
        \n\
        void main()\n\
        {\n\
            fragColour = texture(tex_sampler, tex_coord);\n\
        }\n\
    ";

    void texture_parameters(GLuint tex)
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
};
// @/

### Event Handling

```cpp
// @='poll event queue and handle events'
static SDL_Event ev;
while (SDL_PollEvent(&ev)) switch (ev.type)
{
case SDL_QUIT:
case SDL_APP_TERMINATING:
case SDL_APP_LOWMEMORY:
    context.quit = true;
    break;

@{handle window events}

@{handle keyboard events}

@{handle mouse events}

default:
    break;
}
// @/

// @='handle window events'
    case SDL_WINDOWEVENT:
        // TODO
        break;
// @/

// @='handle keyboard events'
    case SDL_KEYDOWN:
        context.C = 10; 
        context.redraw = true;
        break;

    case SDL_KEYUP:
        context.C = 0; 
        context.redraw = true;
        break;
// @/

// @='handle mouse events'
    case SDL_MOUSEMOTION:
        context.mouse = {ev.button.x / (Scalar)context.w, ev.button.y / (Scalar)context.h};
        if (context.grabbed)
        {
            context.grabbed->s = context.mouse;
#           ifndef __EMSCRIPTEN__
            context.redraw = true;
#           endif
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        context.mouse = {ev.button.x / (Scalar)context.w, ev.button.y / (Scalar)context.h};
        {
            Scalar dist, min_dist;
            min_dist = std::numeric_limits<Scalar>::max();
            for (unsigned int n = 0; n < context.N; ++n)
            {
                auto& d = context.demo[n];
                dist = (context.mouse - d.s).norm();
                if (dist < min_dist) 
                {
                    context.grabbed = &d;
                    context.grabbed_idx = n;
                    min_dist = dist;
                }
            }
            if (min_dist > context.grab_dist / (Scalar)context.w) context.grabbed = nullptr;
        }
        if (context.C) context.redraw = true;
        break;

    case SDL_MOUSEBUTTONUP:
        context.grabbed = nullptr;
        context.redraw = true;
        break;

    case SDL_MOUSEWHEEL:
        context.active_interpolator = (context.active_interpolator + 1) % context.num_interpolators;
        context.redraw = true;
        break;
// @/
```

### Setup

```cpp
// @='setup'
@{SDL setup}

@{initialize random demonstrations}

@{resize interpolators extra lists}

@{create shader programs}

@{create vector buffer objects}

@{create main texture}

atexit(cleanup);
// @/

// @+'globally visible state'
SDL_Window * window = nullptr;
SDL_GLContext gl = nullptr;
// @/

// @='SDL setup'
if (SDL_Init(SDL_INIT_VIDEO) != 0)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Error initializing SDL:\n    %s\n", 
            SDL_GetError());
    return EXIT_FAILURE;
}
else SDL_Log("Initialized successfully\n");

SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
context.window = SDL_CreateWindow
        ( "Interpolators"
        , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED
        , context.w , context.h
        , SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN
        );
if (context.window == nullptr)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Error creating window:\n    %s\n", 
            SDL_GetError());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
            "Couldn't create the main window :(", NULL);
    return EXIT_FAILURE;
}
else SDL_Log("Created window\n");

context.gl = SDL_GL_CreateContext(context.window);
if (context.gl == nullptr)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Error creating OpenGL context:\n    %s\n", 
            SDL_GetError());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
            "Couldn't create OpenGL context :(", NULL);
    return EXIT_FAILURE;
}
else SDL_Log("Created GL context\n");
// @/

// @+'helper functions'
void cleanup ()
{
    glDeleteTextures(1, &context.texture_gl);
    glDeleteBuffers(1, &context.screen_quad_vbo);
    glDeleteProgram(context.prog);
    SDL_GL_DeleteContext(context.gl);
    SDL_DestroyWindow(context.window);
    SDL_Quit();
}
// @/
```

```cpp
// @='initialize random demonstrations'
unsigned int n = context.N;
unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine generator (seed);
std::uniform_real_distribution<Scalar> random(0, 1);
while(n-- > 0)
{
    auto v = Vec2{random(generator), random(generator)};
    auto c = RGB_to_JzAzBz(RGBVec{random(generator), random(generator), random(generator)});
    context.demo.push_back({n, v, c});
}
// @/

// @='resize interpolators extra lists'
auto resize_lists = [&](auto& tup)
{
    auto& meta = std::get<1>(tup);
    auto& para = std::get<2>(tup);
    auto& default_para = std::get<3>(tup);
    meta.resize(context.demo.size());
    for (auto& m : meta) m = {};
    para.resize(context.demo.size());
    for (auto& p : para) p = default_para;
};
std::apply([&](auto& ... tuples) {((resize_lists(tuples)), ...);}, interpolators);
// @/
```

```cpp
// @+'helper functions'
template<typename ShaderProgram, GLenum shader_type>
GLuint create_shader()
{
    GLuint shader;
    const char * source;
    const char * name;
    if constexpr (shader_type == GL_VERTEX_SHADER)
    {
        shader = glCreateShader(shader_type);
        source = ShaderProgram::vert;
        name = ShaderProgram::vert_name;
    }
    else if constexpr (shader_type == GL_FRAGMENT_SHADER)
    {
        shader = glCreateShader(shader_type);
        source = ShaderProgram::frag;
        name = ShaderProgram::frag_name;
    }
    glShaderSource(shader, 1, (const GLchar**) &source, NULL);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (not compiled)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader compilation failed: %s.\n", name);
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        GLchar * errLog = (GLchar*)malloc(logLength);
        if (errLog)
        {
            glGetShaderInfoLog(shader, logLength, &logLength, errLog);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", errLog);
            free(errLog);
        }
        else SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get shader log.\n");
    
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

template<typename ShaderProgram>
GLuint create_program()
{
    const char * prog_name = ShaderProgram::name;

    GLuint vertShader = create_shader<ShaderProgram, GL_VERTEX_SHADER>();
    if (vertShader == 0) return 0;
    GLuint fragShader = create_shader<ShaderProgram, GL_FRAGMENT_SHADER>();
    if (fragShader == 0)
    {
        glDeleteShader(vertShader);
        return 0;
    }

    GLuint program = glCreateProgram();
    if (not program)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create shader program: %s.\n", prog_name);
    }
    
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (not linked)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader linking failed: %s.\n", prog_name);
        GLint logLength = 0;
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &logLength);
        GLchar * errLog = (GLchar*)malloc(logLength);
        if (errLog)
        {
            glGetProgramInfoLog(program, logLength, &logLength, errLog);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", errLog);
            free(errLog);
        }
        else SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get program log.\n");
    
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    return program;
}
// @/
```

```cpp
// @+'globally visible state'
GLuint prog = 0;
// @/

// @='create shader programs'
context.prog = create_program<TextureQuad>();
if (not context.prog) return EXIT_FAILURE;
glUseProgram(context.prog);
// @/

// @+'helper functions'
template<typename Vertex>
GLuint create_vbo(const Vertex * vertices, GLuint numVertices)
{
    GLuint vbo;
    int nBuffers = 1;
    glGenBuffers(nBuffers, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        glDeleteBuffers(nBuffers, &vbo);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "VBO creation failed with code `%u`.\n", err);
        vbo = 0;
    }

    return vbo;
}
// @/

// @+'globally visible state'
const std::vector<Vec2> screen_quad = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };
GLuint screen_quad_vbo = 0;
// @/

// @='create vector buffer objects'
context.screen_quad_vbo = create_vbo(context.screen_quad.data(), context.screen_quad.size());
if (not context.screen_quad_vbo) return EXIT_FAILURE;

GLuint positionIdx = 0;
glBindBuffer(GL_ARRAY_BUFFER, context.screen_quad_vbo);
glVertexAttribPointer(positionIdx, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (const GLvoid*)0);
glEnableVertexAttribArray(positionIdx);
// @/
```

```cpp
// @+'helper functions'
bool write_gl_texture(const Texture& mat, GLuint tex)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F
                , mat.cols(), mat.rows(), 0
                , GL_RGBA , GL_FLOAT, mat.data());
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        glDeleteBuffers(1, &tex);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate memory for texture.\n");
        return false;
    }
    return true;
}

GLuint create_gl_texture(const Texture& mat)
{
    GLuint tex;
    glGenTextures(1, &tex);
    if (not write_gl_texture(mat, tex)) return 0;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}
// @/
```

```cpp
// @+'globally visible state'
Texture texture;
GLuint texture_gl = 0;
// @/

// @='create main texture'
context.texture = Texture(context.h, context.w);
context.texture_gl = create_gl_texture(context.texture);
if (not context.texture_gl) return EXIT_FAILURE;

glUseProgram(context.prog);
GLint tex_sampler_uniform_location = glGetUniformLocation(context.prog, "tex_sampler");
if (tex_sampler_uniform_location < 0) 
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get 'tex_sampler' uniform location.\n");
    return EXIT_FAILURE;
}
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, context.texture_gl);
glUniform1i(tex_sampler_uniform_location, 0);
// @/
```

### Includes

```cpp
// @+'includes'
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <vector>
#include <tuple>
#include <random>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <SDL.h>
#include <SDL_log.h>
#include <SDL_error.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_events.h>
#include <SDL_opengles2.h>
#include <GLES3/gl3.h>
#include <Eigen/Core>
#include <Eigen/LU>
#include "../interpolator/marier_spheres.h"
// @/

//@+'globally visible state'
    std::vector<Interpolator::Demo> demo;
    std::size_t N = 3; // number of demonstrations
    const std::size_t num_interpolators = std::tuple_size_v<decltype(interpolators)>;
    unsigned int w = 500;
    unsigned int h = 500;
    bool redraw = true;
    bool quit = false;
    Scalar grab_dist = 20;
    Interpolator::Demo * grabbed = nullptr;
    std::size_t grabbed_idx = 0;
    Vec2 mouse = {0, 0};
// @/
```

## Converting RGB to perceptually uniform colour space

As mentioned above, the interpolator in the drawing example uses a perceptually
uniform colour space called J_zA_zB_z to ensure that the topology of the
interpolator is well represented by the image without distortion by the
non-linear relationship of the colour representation to typical human colour
vision. Since the colours are rendered in RGB, it is necessary to convert RGB
to and from J_zA_zB_z It is assumed that the RGB colour values will be
interpreted and displayed by the system as
[sRGB](https://en.wikipedia.org/wiki/SRGB), a reasonable default assumption due
to the ubiquitous use of this standard colour space on the web, OpenGL, and
other standards and consumer optical equipment.  Both sRGB and J_zA_zB_z colour
spaces are defined relative to [CIE
XYZ](https://en.wikipedia.org/wiki/CIE_1931_color_space) colour space, a
standard colour space defined by the International Commission on Illumination
in 1931 that defines a quantitative representation of the physiological
response of the eye as it relates to the perception of a certain colour. Given
this shared root, converting to or from sRGB to J_zA_zB_z requires first
converting to or from CIE XYZ.  We assume that the sRGB values are normalized
between 0.0 and 1.0, that the CIE XYZ Y value ranges between 0.0 and 1.0.

The conversion between sRGB and CIE XYZ is given in [the wikipedia page on
sRGB](https://en.wikipedia.org/wiki/SRGB). Since the example programs make use
of Eigen for their colour data, the conversion can be succinctly implemented
with matrix multiplication and other Eigen features.

```cpp
// @='RGB - XYZ colour conversions'
RGBVec XYZ_to_RGB(const CIEXYZVec& xyz)
{
    RGBVec rgb;
    Eigen::Matrix3f a;
    a <<  3.24096994, -1.53738318, -0.49861076,
         -0.96924364,  1.8759675,   0.04155506,
          0.05563008, -0.20397696,  1.05697151;

    rgb = a * xyz; // apply linear transformation

    for (unsigned int i = 0; i < 3; i++) // apply gamme correction
    {
        auto u = rgb[i];
        u = u > 0.0031308 ? (1.055 * pow(u, 1 / 2.4) - 0.055) : 12.92 * u;
        u = u < 0.0 ? 0.0 : u;
        u = u > 1.0 ? 1.0 : u;
        rgb[i] = u;
    }

    return rgb;
}

CIEXYZVec RGB_to_XYZ(const RGBVec& rgb)
{
    CIEXYZVec xyz = rgb;
    Eigen::Matrix3f a;
    a <<  0.41239080,  0.35758434,  0.18048079,
          0.21263901,  0.71516868,  0.07219232,
          0.01933082,  0.11919478,  0.95053215;

    for (unsigned int i = 0; i < 3; i++) // reverse gamme correction
    {
        auto u = xyz[i];
        u = u > 0.04045 ? pow((u + 0.055) / 1.055, 2.4) : u / 12.92;
        xyz[i] = u;
    }

    return a * xyz; // reverse linear transformation
}
// @/
```

The conversion between J_zA_zB_z and CIE XYZ is given in [the 2017 paper by Safdar
et al. introducing J_zA_zB_z published by the Optical Society of America in Volume
25 Issue 13 pages 15131-15151 of the journal Optics
Express](https://doi.org/10.1364/OE.25.015131).

```cpp
// @='JzAzBz consts'
static const Eigen::Matrix3f M1 = (Eigen::Matrix3f() <<
    0.41478972,  0.579999,  0.0146480,
    -0.2015100,  1.120649,  0.0531008,
    -0.0166008,  0.264800,  0.6684799).finished();
static const Eigen::Matrix3f M2 = (Eigen::Matrix3f() <<  
    0.5,         0.5,       0.0,
    3.524000,   -4.066708,  0.542708,
    0.199076,    1.096799, -1.295875).finished();
constexpr float two5 = 1 << 5;
constexpr float two7 = 1 << 7;
constexpr float two12 = 1 << 12;
constexpr float two14 = 1 << 14;
constexpr float b = 1.15;
constexpr float g = 0.66;
constexpr float c1 = 3424.0 / two12;
constexpr float c2 = 2413.0 / two7;
constexpr float c3 = 2392.0 / two7;
constexpr float n = 2610.0 / two14;
constexpr float p = 1.7 * 2523.0 / two5;
constexpr float d = -0.56;
constexpr float d_0 = 1.6295499532821566e-11;
// @/

// @='JzAzBz - XYZ colour conversions'
JzAzBzVec XYZ_to_JzAzBz(const CIEXYZVec& xyz)
{
    @{JzAzBz consts}
    auto x = xyz.x();
    auto y = xyz.y();
    auto z = xyz.z();

    // pre-adjust to improve iso-hue linearity (Safdar et al. eqn. 8
    auto x_ = b * x - (b - 1.0) * z;
    auto y_ = g * y - (g - 1.0) * x;
    CIEXYZVec xyz_(x_, y_, z);

    // transform xyz to cone primaries (Safdar et al. eqn. 9)
    Eigen::Vector3f lms = M1 * xyz_;

    // perceptual quantizer (Safdar et al. eqn. 10
    for (unsigned int i = 0; i < 3; ++i)
    {
        auto u = pow(lms[i] / 10000.0, n);
        lms[i] = pow( (c1 + c2 * u) / (1.0 + c3 * u), p );
    }

    // transform to correlates of opponent colour space (Safdar et al. eqn. 11)
    Eigen::Vector3f jab = M2 * lms;

    // improve wide-range lightness prediction (Safdar et al. eqn. 12)
    auto i = jab[0];
    jab[0] = ((1.0 + d) * i) / (1.0 + d * i) - d_0;

    return jab;
}

CIEXYZVec JzAzBz_to_XYZ(const JzAzBzVec& jab)
{
    @{JzAzBz consts}
    static const Eigen::Matrix3f M1inv = M1.inverse();
    static const Eigen::Matrix3f M2inv = M2.inverse();

    // eqn. 17
    auto iab = jab;
    auto j = jab[0] + d_0;
    iab[0] = j / (1.0 + d - d * j);

    // eqn. 18
    Eigen::Vector3f lms = M2inv * iab;

    // eqn. 19
    for (unsigned int i = 0; i < 3; ++i)
    {
        auto u = pow(lms[i], 1.0/p);
        lms[i] = 10000.0 * pow( (c1 - u) / (c3 * u - c2), 1.0/n );
    }

    // eqn. 20
    auto xyz_ = M1inv * lms;

    // eqn. 21 - 23
    auto x_ = xyz_[0];
    auto y_ = xyz_[1];
    auto z_ = xyz_[2];
    auto x = (x_ + (b - 1.0) * z_) / b;
    auto y = (y_ + (g - 1.0) * x) / g;
    auto z = z_;

    CIEXYZVec xyz{x, y, z};
    return xyz;
}
// @/
```

The conversion between sRGB and J_zA_zB_z is then easily implemented in terms
of the above conversions to and from CIE XYZ colour space:

```cpp
// @='colour conversions'
@{RGB - XYZ colour conversions}

@{JzAzBz - XYZ colour conversions}

JzAzBzVec RGB_to_JzAzBz(const RGBVec& rgb)
{
    return XYZ_to_JzAzBz(RGB_to_XYZ(rgb));
}

RGBVec JzAzBz_to_RGB(const JzAzBzVec& jab)
{
    return XYZ_to_RGB(JzAzBz_to_XYZ(jab));
}
// @/
```

## Compiling Examples

A simple Makefile is included in this repository with rules for building the
demo app, as well as for generating the machine source code and pretty-printed
source code from this literate source code document using
[`lilit`](https://github.com/DocSunset/lilit). The Makefile is tested and
working on the author's Arch Linux system. As usual, I am open to
recommendations and pull requests to improve the compatibility of the Makefile,
or on other ways of building for various platforms.
