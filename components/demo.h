#pragma once
#include <Eigen/Dense>
#include <entt/entt.hpp>

namespace Component
{
    struct Demo
    {
        static constexpr std::size_t num_sources = 2;
        static constexpr std::size_t num_destinations = 6;

        using Source = Eigen::Matrix<double, num_sources, 1>;
        using Destination = Eigen::Matrix<double, num_destinations, 1>;

        entt::entity id;
    };
}
