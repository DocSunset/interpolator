#pragma once
#include <entt/entt.hpp>
#include "components/demo.h"
#include "components/pca.h"

namespace Private::PCA
{
    void regenerate_pca(entt::registry&, entt::entity);
    void reset(Component::SourcePCA&);
    void reset(Component::DestinationPCA&);
    void _analyse(Component::SourcePCA&, const Eigen::MatrixXf&);
    void _analyse(Component::DestinationPCA&, const Eigen::MatrixXf&);

    struct SourceDataset
    {
        static constexpr std::size_t Cols = Component::Demo::num_sources;
        using Vector = Component::Demo::Source;
        using Matrix = Eigen::Matrix<float, Eigen::Dynamic, Cols>;
        Matrix matrix;
    };

    struct DestinationDataset
    {
        static constexpr std::size_t Cols = Component::Demo::num_destinations;
        using Vector = Component::Demo::Destination;
        using Matrix = Eigen::Matrix<float, Eigen::Dynamic, Cols>;
        Matrix matrix;
    };
}
