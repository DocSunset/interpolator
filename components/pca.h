#pragma once
#include "demo.h"

namespace Component
{
    template<std::size_t _original, std::size_t _reduced> struct _PCA
    {
        static constexpr std::size_t Original() { return _original; }
        static constexpr std::size_t Reduced() { return _reduced; }
        Eigen::Matrix<double, _original, 1> mean;
        Eigen::Matrix<double, _reduced, _original> projection;
        Eigen::Matrix<double, _original, _reduced> inverse_projection;
    };

    struct SourcePCA : public _PCA<Demo::num_sources, 2> {};
    struct DestinationPCA : public _PCA<Demo::num_destinations, 3> {};
}
