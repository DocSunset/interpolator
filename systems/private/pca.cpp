#include "pca.h"
#include <Eigen/SVD>
#include "components/update.h"
#include "components/pca.h"
#include "components/paint_flag.h"

namespace
{
    template<typename Dataset>
    auto& update_dataset(entt::registry& registry, entt::entity entity)
    {
        auto& dataset = registry.ctx<Dataset>();
        // TODO: granularly update dataset based on updated demo in entity
        // for now, just copy all of the vectors
        auto view = registry.view<Component::Demo>();
        std::size_t size = 0;
        for (auto d : view) if (not registry.get<Component::Demo>(d).destroyed) ++size;
        dataset.matrix.resize(size, Dataset::Cols);
        std::size_t row = 0;
        for (auto d : view)
        {
            if (registry.get<Component::Demo>(d).destroyed) continue;

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

        //std::cout << "**********analysis***********\n";
        //std::cout << dataset << "\n\n";
        //std::cout << pca.mean << "\n\n";

        Eigen::JacobiSVD<Eigen::Matrix<double, Eigen::Dynamic, PCA::Original()>>
            svd(dataset.rowwise() - dataset.colwise().mean(), Eigen::ComputeFullV);
        auto V = svd.matrixV().leftCols(PCA::Reduced());

        //std::cout << V << "\n\n";
        //std::cout << svd.singularValues() << "\n\n";
        Eigen::MatrixXd S = svd.singularValues()(Eigen::seqN(0,PCA::Reduced()));
        //std::cout << S << "\n\n";
        S = (S.array() + 0.000001).sqrt(); // array + arbitrary epsilon to avoid divide by zero errors
        //std::cout << S << "\n\n";


        Eigen::MatrixXd Sinv = S.array().inverse();
        pca.projection = Sinv.asDiagonal() * V.transpose();
        pca.inverse_projection = V * S.asDiagonal();
        
        //std::cout << pca.projection << "\n\n";
        //for (auto& row : dataset.rowwise())
        //    std::cout << pca.projection * row.transpose() << "\n";
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
        registry.replace<PCA>(registry.ctx<Dataset>().entity, pca);
        registry.ctx<Component::PaintFlag>().set();
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

