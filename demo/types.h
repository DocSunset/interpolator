#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include "../include/interpolators.h"

using Scalar = float;
using ID = unsigned int;
using Vec2 = Eigen::Vector2f;
using RGBVec = Eigen::Vector3f;
using CIEXYZVec = Eigen::Vector3f;
using JzAzBzVec = Eigen::Vector3f;

using Demo = Interpolators::Demo<Scalar, ID, Vec2, JzAzBzVec>;
using DemoList = std::vector<Demo>;
template<typename Interpolator>
using Shadr = ShaderInterpolators::AcceleratedInterpolator<Interpolator>;

#define INTERPOLATOR(type, ...) std::make_tuple(type{}, std::vector<type::Meta>{}, std::vector<type::Para>{}, type::Para{__VA_ARGS__}, Shadr<type>{})
auto interpolators = std::make_tuple
        ( //INTERPOLATOR(Interpolators::IntersectingNSpheres<Demo>)
        /*,*/ INTERPOLATOR(Interpolators::InverseDistance<Demo>, 4, 0.001, 0.0, 1.0)
        );

const std::size_t num_interpolators = std::tuple_size_v<decltype(interpolators)>;

#endif
