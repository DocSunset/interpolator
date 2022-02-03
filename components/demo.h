#pragma once
#include <Eigen/Dense>
#include <entt/entt.hpp>
#include "position.h"
#include "fmsynth.h"

namespace Component
{
    struct Demo
    {
        static constexpr std::size_t num_sources = 2;
        static constexpr std::size_t num_destinations = 6;

        using Source = Eigen::Matrix<float, num_sources, 1>;
        using Destination = Eigen::Matrix<float, num_destinations, 1>;

        entt::entity id;
    };
}
