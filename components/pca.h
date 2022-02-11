#pragma once
#include "demo.h"

namespace Component
{
    template<std::size_t _original, std::size_t _reduced> struct _PCA
    {
        static constexpr std::size_t Original = _original;
        static constexpr std::size_t Reduced = _reduced;
        Eigen::MatrixXf mean;
        Eigen::MatrixXf projection;
        Eigen::MatrixXf inverse_projection;
    };

    struct SourcePCA : public _PCA<Demo::num_sources, 2> {};
    struct DestinationPCA : public _PCA<Demo::num_destinations, 3> {};
}
