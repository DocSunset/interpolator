#pragma once
#include "position.h"
#include "fmsynth.h"

namespace Component
{
    struct Demo
    {
        int i;
        Position * source;
        FMSynthParameters * destination;
    };

    const Position& source(const Demo& d);
          Position& source(Demo& d);
    const Position& destination(const Demo& d);
          Position& destination(Demo& d);
}
