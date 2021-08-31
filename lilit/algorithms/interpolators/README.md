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
including the ones presented here, is for the user to specify pairs of
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

The main challenge in the implementation of the interpolators is how best to
represent the data on which they operate, how to marshal the data into and out
of the algorithm, and what data to expose. Some of the algorithms (such as the
intersecting N-spheres algorithm) can operate using only the list of
demonstrations.  Others (such as inverse weighted distance) may use additional
attributes related to each demonstration.  All of the algorithms output a
weighted sum, but most also calculate various other points of metadata that
may be of interest to the user, or useful for visualizing the internal
operation of the algorithm.

In order to simplify the implementations here, I made a number of guiding
decisions:

- don't bother outputting meta-data
- forbid point-wise attributes

With these simplifications made, all algorithms can be reduced to the same
interface: a single function accepting

that could be considered as trading in versatility in exchange for
reduced complexity. I will argue however that what is lost by these choices is
inconsequential compared to the huge increase in complexity that would be required
otherwise, especially since it is one goal of the library to remain simple and
easy to adopt, which is not possible to acheive if complex opinionated solutions
have to be found to deeply challenging problems.
