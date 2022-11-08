#define EIGEN_USE_MKL_ALL
#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>

#include "diffuse.hpp"

PYBIND11_MODULE(diffuse_py, m) {
    // m.doc() = "pybind11 example plugin"; // optional module docstring

    m.def("diffuse", &diffuse, "A function that diffuses masked values");
}
