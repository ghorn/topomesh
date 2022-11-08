#!/usr/bin/env python3

"""
diffuse elements that don't have data
"""

from scipy.sparse import coo_matrix
from scipy.sparse.linalg import spsolve
# from scikits.umfpack import spsolve
import argparse
from pathlib import Path
import numpy as np
from PIL import Image
import time

import diffuse_binding.diffuse_binding as diffuse_binding

# disable DecompressionBomb protection
Image.MAX_IMAGE_PIXELS = None


def diffuse(original_pixels):
    """
    diffuse elements that don't have data
    """
    return diffuse_binding.diffuse(original_pixels)

    n_rows, n_cols = original_pixels.shape

    index_map = {}
    # constant_map = {}

    print('forming index map...')
    for row_idx in range(n_rows):
        for col_idx in range(n_cols):
            if original_pixels[row_idx, col_idx] == 0:
                index_map[(row_idx, col_idx)] = len(index_map)
            # else:
            #     constant_map[(row_idx, col_idx)] = original_pixels[row_idx, col_idx]

    num_vars = len(index_map)
    print(f"number of variables: {num_vars}")

    print('forming A and b...')
    bs = np.zeros(num_vars)

    # initialize to 5x the number of variables, which is more than needed
    # some variables don't have 4 neighbors
    # The point is to pre-initialize the arrays to a size that is large enough
    # to avoid allocation in a loop
    A_row = np.empty(5*num_vars, dtype=np.int32)
    A_col = np.empty(5*num_vars, dtype=np.int32)
    A_data = np.empty(5*num_vars, dtype=np.float64)


    nnz = 0

    for (row_idx, col_idx), this_variable_index in index_map.items():
        num_neighbors = 0

        # get the neighbors and compute number of neighbors
        neighbor_indices = []
        if row_idx > 0:
            neighbor_indices.append((row_idx - 1, col_idx))
        if col_idx > 0:
            neighbor_indices.append((row_idx, col_idx - 1))
        if row_idx < n_rows - 1:
            neighbor_indices.append((row_idx + 1, col_idx))
        if col_idx < n_cols - 1:
            neighbor_indices.append((row_idx, col_idx + 1))

        num_neighbors = len(neighbor_indices)

        # neighbor indices
        b = 0.

        # the variable itself
        A_row[nnz] = this_variable_index
        A_col[nnz] = this_variable_index
        A_data[nnz] = 1.
        nnz += 1

        # the neighbor contributions
        for neighbor_index in neighbor_indices:
            if neighbor_index in index_map:
                A_row[nnz] = this_variable_index
                A_col[nnz] = index_map[neighbor_index]
                A_data[nnz] = -1. / num_neighbors
                nnz += 1
            else:
                b += original_pixels[neighbor_index[0], neighbor_index[1]] / num_neighbors

        bs[this_variable_index] = b

    print('converting A to coo_matrix...')
    # discard unused entries in the arrays
    A_row = A_row[:nnz]
    A_col = A_col[:nnz]
    A_data = A_data[:nnz]

    A = coo_matrix((A_data, (A_row, A_col)), shape=(num_vars, num_vars))

    print('converting A to csc_matrix...')
    A = A.tocsc()

    print('solving...')
    x = spsolve(A, bs)

    print('reconstructing image...')
    # form the output image
    pixels = original_pixels.copy()
    for (row_idx, col_idx), var_index in index_map.items():
        pixels[row_idx, col_idx] = x[var_index]

    return pixels


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('infile', type=Path)
    parser.add_argument('outfile', type=Path)
    args = parser.parse_args()
    print(args)

    print(f'loading {args.infile}...')
    im = Image.open(args.infile)
    width, height = im.size

    print(im)
    print(f'width * height = {width} * {height} = {width * height}')

    # print('resizing...')
    # im.resize((int(width*0.1), int(height*0.1)))

    print('extracting pixels...')
    pixels = np.array(im.getdata()).reshape((height, width))
    print(f'min value = {pixels.min()}')
    print(f'max value = {pixels.max()}')
    print(pixels)
    print(pixels.shape)
    print(pixels.dtype)

    print('converting to float...')
    pixels = pixels.astype(np.float64)

    print('diffusing...')
    t0 = time.time()
    pixels = diffuse(pixels)
    t1 = time.time()
    print(f"diffused in {t1 - t0} seconds")

    print(f'saving {args.outfile}...')
    im_out = Image.fromarray(pixels).convert('I')
    im_out.save(args.outfile)

    # im.show()

if __name__ == '__main__':
    main()
