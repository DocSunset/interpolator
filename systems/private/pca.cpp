#include "pca.h"
#include <iostream>
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
        dataset.matrix.resize(view.size(), Dataset::Cols);
        std::size_t row = 0;
        for (auto d : view)
        {
            const auto& v = registry.get<typename Dataset::Vector>(d);
            std::size_t col = 0;
            for (auto f : v)
            {
                dataset.matrix(row, col) = f;
                ++col;
            }
            ++row;
        }
        return dataset.matrix;
    }

    template<typename PCA>
    void _analyse(PCA& pca, const Eigen::MatrixXd& dataset)
    {
        pca.mean = dataset.colwise().mean();
        Eigen::JacobiSVD<Eigen::Matrix<double, Eigen::Dynamic, PCA::Original()>>
            svd(dataset.rowwise() - dataset.colwise().mean(), Eigen::ComputeFullV);
        auto V = svd.matrixV().leftCols(PCA::Reduced());
        Eigen::VectorXd S = svd.singularValues();
        S = S(Eigen::seqN(0,PCA::Reduced()));
        S = S.array().sqrt();
        S = S * 0.5;

        std::cout << "Dataset dims: " << dataset.rows() << " " << dataset.cols() << "\n";
        std::cout << "V:\n" << V << "\n\n";
        std::cout << "S:\n" << S << "\n\n";

        pca.projection = S.asDiagonal() * V.transpose();
        S = S.array().inverse();
        pca.inverse_projection = V * S.asDiagonal();

        std::cout << "Projection:\n" << pca.projection << "\n\n";
    }

    template<typename PCA>
    void reset(PCA& pca)
    {
        pca.mean = pca.mean.Zero();
        pca.projection = pca.projection.Zero();
        pca.inverse_projection = pca.inverse_projection.Zero();
        for (std::size_t i = 0; i < pca.Reduced(); ++i)
        {
            pca.projection(i,i) = 1;
            pca.inverse_projection(i,i) = 1;
        }
    }

    template<typename PCA, typename Dataset>
    void regenerate(entt::registry& registry, entt::entity entity)
    {
        auto& pca = registry.ctx<PCA>();
        const auto& dataset = update_dataset<Dataset>(registry, entity);
        if (dataset.rows() < 3)
        {
            reset(pca);
        }
        else _analyse(pca, dataset);
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

    void _analyse(Component::SourcePCA& pca, const Eigen::MatrixXd& dataset) { return ::_analyse(pca, dataset); }
    void _analyse(Component::DestinationPCA& pca, const Eigen::MatrixXd& dataset) { return ::_analyse(pca, dataset); }
    void reset(Component::SourcePCA& pca) { return ::reset(pca); }
    void reset(Component::DestinationPCA& pca) { return ::reset(pca); }

}

