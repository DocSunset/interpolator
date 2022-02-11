#include "pca.h"
#include <Eigen/SVD>
#include "components/update.h"
#include "components/pca.h"

namespace
{
    template<typename Dataset>
    auto& update_dataset(entt::registry& registry, entt::entity entity)
    {
        auto& dataset = registry.ctx<Dataset>();
        // TODO: granularly update dataset based on updated demo in entity
        // for now, just copy all of the vectors
        auto view = registry.view<Component::Demo>();
        dataset.matrix.resize(Dataset::Rows, view.size());
        std::size_t col = 0;
        for (auto d : view)
        {
            const auto& v = registry.get<typename Dataset::Vector>(d);
            std::size_t row = 0;
            for (auto f : v)
            {
                dataset.matrix(row, col) = f;
                ++row;
            }
            ++col;
        }
        return dataset.matrix;
    }

    template<typename PCA, typename Dataset>
    void regenerate(entt::registry& registry, entt::entity entity)
    {
        auto& pca = registry.ctx<PCA>();
        const auto& dataset = update_dataset<Dataset>(registry, entity);
        if (dataset.cols() < 3) return;
        pca.mean = dataset.rowwise().mean();
        Eigen::JacobiSVD<typename Dataset::Matrix> svd(dataset.colwise() - dataset.rowwise().mean()
                , Eigen::ComputeThinV);
        pca.projection = svd.matrixV().leftCols(PCA::Reduced);
        pca.inverse_projection = pca.projection.transpose();
    }
}

namespace Private::PCA
{
    void regenerate_pca(entt::registry& registry, entt::entity entity)
    {
        const auto& update = registry.get<Component::Update>(entity);
        switch (update.type)
        {
            case Component::Update::Type::Insert:
            case Component::Update::Type::Delete:
                regenerate<Component::SourcePCA, SourceDataset>(registry, update.entity);
                regenerate<Component::DestinationPCA, DestinationDataset>(registry, update.entity);
                break;
            case Component::Update::Type::Source:
                regenerate<Component::SourcePCA, SourceDataset>(registry, update.entity);
                break;
            case Component::Update::Type::Destination:
                regenerate<Component::DestinationPCA, DestinationDataset>(registry, update.entity);
                break;
            default:
                break;
        }
    }
}

