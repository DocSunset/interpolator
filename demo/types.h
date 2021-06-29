#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include "../include/interpolators.h"

using Scalar = float;
using ID = unsigned int;
using Vec2 = Eigen::Vector2f;
using RGBVec = Eigen::Vector3f;

using Demo = Interpolators::Demo<Scalar, ID, Vec2, RGBVec>;
using DemoList = std::vector<Demo>;
template<typename Interpolator>
using Shadr = ShaderInterpolators::AcceleratedInterpolator<Interpolator>;

#endif
