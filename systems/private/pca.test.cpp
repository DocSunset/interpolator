#include <catch2/catch.hpp>
#include <iostream>
#include <entt/entt.hpp>
#include "pca.h"

TEST_CASE("PCA computes according to expectation", "[pca]")
{
    Component::SourcePCA pca;

    SECTION("PCA of aligned unit vectors gives identity matrix")
    {
        Eigen::MatrixXf dataset(4,6);
        Eigen::MatrixXf expected(2,6);
        dataset << 1,0,0,0,0,0,
                   0,1,0,0,0,0,
                   0,-1,0,0,0,0,
                   -1,0,0,0,0,0;

        std::cout << "Dataset:\n";
        std::cout << dataset << "\n\n";

        Private::PCA::_analyse(pca, dataset);

        std::cout << "Mean:\n"
                  << pca.mean << "\n\n"
                  << "Projection:\n"
                  << pca.projection << "\n\n";
    }
}
