# Mapping Algorithms

This repository hosts a collection of mapping algorithms implemented in C++,
as well as an application demonstrating and facilitating exploration of their
use.

My goal with the implementations is to help encourage the adoption of the most
exciting techniques presented in the research literature. In order to acheive
this, I aspire to these goals:

- the implementations should be easy to adopt
    - this implies lightweight, minimum dependencies, easy to understand, well
      documented, etc.
- the implementations should be independent
    - it should be simple to use just one of these algorithms without drawing
      in all the others
    - this is meant to be a library of libraries, not a single monolithic package
- the implementations should be mutually compatible
    - taking the time to learn how one of the algorithms works should make it
      trivial to use any of them
    - this may not extend to algorithms which are fundamentally different in nature...

The code is written in a literate style using
[lilit](https://github.com/docsunset/lilit). The pretty printed source code can
be browsed through github, as the literate sources are just markdown with
embedded c++.  [The literate sources for the algorithms are presented in the
`lilit/algorithms` directory](lilit/algorithms/README.md).  The machine sources
are tangled into the `include` directory.

In addition to the algorithms themselves, the demo application facilitates
exploring and comparing the different algorithms. It serves as a prototyping
environment for testing novel interaction modalities and approaches to using
these algorithms, and also works as a kind of general integration test for the
algorithms.  [The literate sources for the demo app are located in the
`lilit/demo` directory](lilit/demo/README.md).  The machine sources are tangled
into the `demo` directory.

This is work in progress, and the ultimate fate of this code is still
uncertain.  For now, all applicable rights are reserved.  If you would like to
use this code, please contact me at travis at traviswest dot ca.
