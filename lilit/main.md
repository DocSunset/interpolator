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

# Implementation

This section introucing the implementation needs improvement. Please stand by.

The interpolators are all gathered inside of a shared `struct`. This is done in
order to propagate the template parameters for the `Scalar, ID, SVector,` and
`PVector` types to all the interpolators so that in case the calling program
wants to use the same types for all interpolators they only need to declare
them once. The demonstration type, `struct Demo`, is also declared at this
level.

```cpp 
// @#'include/interpolators.h'
#ifndef INTERPOLATORS_H
#define INTERPOLATORS_H

#include<limits>
#include<cmath>
#include<iostream>
#include<cstddef>

@{interpolator convenience macros}

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

    @{interpolators}
};
#endif
// @/
```

@[lilit/intersecting_n_spheres.md]

@[lilit/inverse_weighted_distance.md]

@[lilit/lampshade.md]

@[lilit/nodes.md]

@[lilit/convenience_macros.md]

@[lilit/opengl_rendering.md]

# Interactive Demo

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

A detailed presentation of the implementation of the demo program can be found
by reading its literate source code in the file linked here:

@[lilit/interactive_demo/main.md]

## Installation

This section still needs to be written, but in principle you should be able to
try the demo without installing anything by visiting the site where it is
hosted...

## Usage

This section still needs to be written.

# Building

A simple Makefile is included in this repository with rules for building the
demo app, as well as for generating the machine source code and pretty-printed
source code from this literate source code document using
[`lilit`](https://github.com/DocSunset/lilit). The Makefile is tested and
working on the author's Arch Linux system. As usual, I am open to
recommendations and pull requests to improve the compatibility of the Makefile,
or on other ways of building for various platforms; don't be put off by the use
of a custom literate programming tool. All of the machine source code is
generated simply by running `lilit lilit/main.md` from the root of the
repository, and the machine source code is also included in the repository so
that you don't even need to install lilit to hack on the code and propose
improvements to the build system.
