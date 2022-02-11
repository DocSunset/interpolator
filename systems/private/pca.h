#pragma once
#include <entt/entt.hpp>
#include "components/demo.h"

namespace Private::PCA
{
    void regenerate_pca(entt::registry&, entt::entity);

    struct SourceDataset
    {
        static constexpr std::size_t Rows = Component::Demo::num_sources;
        using Vector = Component::Demo::Source;
        using Matrix = Eigen::Matrix<float, Component::Demo::num_sources, Eigen::Dynamic>;
        Matrix matrix;
    };

    struct DestinationDataset
    {
        static constexpr std::size_t Rows = Component::Demo::num_destinations;
        using Vector = Component::Demo::Destination;
        using Matrix = Eigen::MatrixXf;
        Matrix matrix;
    };
}
