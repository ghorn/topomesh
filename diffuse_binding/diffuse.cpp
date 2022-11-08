#include "diffuse.hpp"

#include <iostream>
// unordered map
#include <unordered_map>
// std::pair
#include <tuple>
#include <utility>
// std::vector
#include <vector>

#include <Eigen/SparseCore>
#include <Eigen/SparseLU>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/OrderingMethods>
#include <Eigen/PardisoSupport>
// #include <Eigen/KLUSupport>


// custom specialization of std::hash can be injected in namespace std
template <typename T1, typename T2>
struct std::hash<std::pair<T1, T2> >
{
    std::size_t operator()(std::pair<T1, T2> const& s) const noexcept
    {
        std::size_t h1 = std::hash<T1>{}(s.first);
        std::size_t h2 = std::hash<T2>{}(s.second);
        return h1 ^ (h2 << 1); // or use boost::hash_combine
    }
};

static void SetupProblem(Eigen::SparseMatrix<double, Eigen::RowMajor> &A, Eigen::VectorXd &bs, const Eigen::MatrixXd &original_pixels, const std::map<std::pair<int32_t, int32_t>, int32_t> &index_map, int32_t num_vars, int32_t n_rows, int32_t n_cols) {
     // # initialize to 5x the number of variables, which is more than needed
    // # some variables don't have 4 neighbors
    // # The point is to pre-initialize the arrays to a size that is large enough
    // # to avoid allocation in a loop
    std::vector<Eigen::Triplet<double>> A_triplets;
    A_triplets.reserve(5 * static_cast<size_t>(num_vars));
    // A_row = np.empty(5*num_vars, dtype=np.int32)
    // A_col = np.empty(5*num_vars, dtype=np.int32)
    // A_data = np.empty(5*num_vars, dtype=np.float64)


    // nnz = 0

    // for (row_idx, col_idx), this_variable_index in index_map.items():
    std::vector<std::pair<int32_t, int32_t> > neighbor_indices;
    for (const auto &index_map_elem : index_map) {
        neighbor_indices.clear();


        const int32_t row_idx = index_map_elem.first.first;
        const int32_t col_idx = index_map_elem.first.second;
        const int32_t this_variable_index = index_map_elem.second;


        //     num_neighbors = 0
        //     # get the neighbors and compute number of neighbors
        //     neighbor_indices = []
        //     if row_idx > 0:
        //         neighbor_indices.append((row_idx - 1, col_idx))
        //     if col_idx > 0:
        //         neighbor_indices.append((row_idx, col_idx - 1))
        //     if row_idx < n_rows - 1:
        //         neighbor_indices.append((row_idx + 1, col_idx))
        //     if col_idx < n_cols - 1:
        //         neighbor_indices.append((row_idx, col_idx + 1))

        //     num_neighbors = len(neighbor_indices)


        if (row_idx > 0) {
            neighbor_indices.push_back(std::make_pair(row_idx - 1, col_idx));
        }
        if (row_idx < n_rows - 1) {
            neighbor_indices.push_back(std::make_pair(row_idx + 1, col_idx));
        }
        if (col_idx > 0) {
            neighbor_indices.push_back(std::make_pair(row_idx, col_idx - 1));
        }
        if (col_idx < n_cols - 1) {
            neighbor_indices.push_back(std::make_pair(row_idx, col_idx + 1));
        }
        const double num_neighbors = static_cast<double>(neighbor_indices.size());

        // neighbor indices
        double b = 0;

        // the variable itself
        //     A_row[nnz] = this_variable_index
        //     A_col[nnz] = this_variable_index
        //     A_data[nnz] = 1.
        //     nnz += 1
        A_triplets.push_back(Eigen::Triplet<double>(this_variable_index, this_variable_index, 1.));

        // the neighbor contributions
        //     for neighbor_index in neighbor_indices:
        //         if neighbor_index in index_map:
        //             A_row[nnz] = this_variable_index
        //             A_col[nnz] = index_map[neighbor_index]
        //             A_data[nnz] = -1. / num_neighbors
        //             nnz += 1
        //         else:
        //             b += original_pixels[neighbor_index[0], neighbor_index[1]] / num_neighbors
        for (const std::pair<int32_t, int32_t> &neighbor_index : neighbor_indices) {
            const auto neighbor_index_it = index_map.find(neighbor_index);
            if (neighbor_index_it != index_map.end()) {
                A_triplets.push_back(Eigen::Triplet<double>(this_variable_index, neighbor_index_it->second, -1. / num_neighbors));
            } else {
                b += original_pixels(neighbor_index.first, neighbor_index.second) / num_neighbors;
            }
        }

        bs(this_variable_index) = b;
    }

    std::cerr << "forming sparse matrix A from triplets..." << std::endl;
    A.setFromTriplets(A_triplets.begin(), A_triplets.end());
}

Eigen::MatrixXd diffuse(const Eigen::MatrixXd &original_pixels) {
    // return original_pixels;

    // std::cerr << "****** using " << Eigen::nbThreads( ) << " threads ******" << std::endl;

    // get number of rows and cols of original_pixels
    const int32_t n_rows = static_cast<int32_t>(original_pixels.rows());
    const int32_t n_cols = static_cast<int32_t>(original_pixels.cols());

    // map of indices
    // std::unordered_map<std::pair<int32_t, int32_t>, int32_t> index_map(index_map_elems);
    std::map<std::pair<int32_t, int32_t>, int32_t> index_map;

    std::cerr << "forming index map..." << std::endl;
    // for row_idx in range(n_rows):
    //     for col_idx in range(n_cols):
    //         if original_pixels[row_idx, col_idx] == 0:
    //             index_map[(row_idx, col_idx)] = len(index_map)
    for (int32_t row_idx = 0; row_idx < n_rows; ++row_idx) {
        for (int32_t col_idx = 0; col_idx < n_cols; ++col_idx) {
            if (original_pixels(row_idx, col_idx) == 0) {
                index_map.insert({std::make_pair(row_idx, col_idx), static_cast<int32_t>(index_map.size())});
            }
        }
    }

    const int32_t num_vars = static_cast<int32_t>(index_map.size());
    std::cerr << "number of variables: " << num_vars << std::endl;

    std::cerr << "forming A and b..." << std::endl;
    Eigen::SparseMatrix<double, Eigen::RowMajor> A(num_vars, num_vars);
    Eigen::VectorXd bs(num_vars);
    SetupProblem(A, bs, original_pixels, index_map, num_vars, n_rows, n_cols);
    // A.makeCompressed();

    Eigen::BiCGSTAB<Eigen::SparseMatrix<double, Eigen::RowMajor>, Eigen::IncompleteLUT<double>> solver;
    // Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
    // Eigen::SparseLU<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int32_t> > solver;
    // Eigen::PardisoLU<Eigen::SparseMatrix<double> > solver;
    // Eigen::PardisoLU<Eigen::SparseMatrix<double, Eigen::RowMajor> > solver;

    std::cerr << "solving (analyzing pattern)..." << std::endl;
    solver.analyzePattern(A);
    if (solver.info() != Eigen::Success) {
        std::cerr << "failed!" << std::endl;
        std::exit(1);
    }
    std::cerr << "solving (factorizing)..." << std::endl;
    solver.factorize(A);
    if (solver.info() != Eigen::Success) {
        std::cerr << "failed!" << std::endl;
        std::exit(1);
    }
    std::cerr << "solving (backsolving)..." << std::endl;
    Eigen::VectorXd x = solver.solve(bs);
    if (solver.info() != Eigen::Success) {
        std::cerr << "failed!" << std::endl;
        std::exit(1);
    }

    std::cerr << "reconstructing image..." << std::endl;
    // form the output image
    Eigen::MatrixXd pixels = original_pixels;
    // for (row_idx, col_idx), var_index in index_map.items():
    //     pixels[row_idx, col_idx] = x[var_index]
    for (const auto &index_map_elem : index_map) {
        const int32_t row_idx = index_map_elem.first.first;
        const int32_t col_idx = index_map_elem.first.second;
        const int32_t var_index = index_map_elem.second;

        pixels(row_idx, col_idx) = x(var_index);
    }


    return pixels;
}
