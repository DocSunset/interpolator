#pragma once
#include "position.h"
#include "fmsynth.h"
#include <Eigen/Dense>

namespace Component
{
    struct Demo
    {
        static constexpr std::size_t num_sources = 2;
        static constexpr std::size_t num_destinations = 6;

        using Source = Eigen::Matrix<float, num_sources, 1>;
        using Destination = Eigen::Matrix<float, num_destinations, 1>;

        long long id;
        Source source;
        Destination destination;

        static Destination zero_destination() {return Destination::Zero();}
    };

    const Demo::Source& source(const Demo& d);
          Demo::Source& source(Demo& d);
    const Demo::Destination& destination(const Demo& d);
          Demo::Destination& destination(Demo& d);
}
