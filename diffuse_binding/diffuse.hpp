#pragma once
#include <cstdint>

#define EIGEN_USE_MKL_ALL
#include <Eigen/Dense>

Eigen::MatrixXd diffuse(const Eigen::MatrixXd &original_pixels);
