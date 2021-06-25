#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include "../include/interpolators.h"

using Scalar = float;
using ID = unsigned int;
using Vec2 = Eigen::Vector2f;
using RGBVec = Eigen::Vector3f;
using RGBAVec = Eigen::Vector4f;
using CIEXYZVec = Eigen::Vector3f;
using JzAzBzVec = Eigen::Vector3f;
using Texture = Eigen::Matrix<RGBAVec, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using Interpolator = Interpolators<Scalar, ID, Vec2, JzAzBzVec>;
using DemoList = std::vector<Interpolator::Demo>;
const std::size_t num_interpolators = 2;
#endif
