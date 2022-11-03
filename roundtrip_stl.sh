#/usr/bin/env bash

set -e

# command line args
roundtrip_stl_binary=$1
test_mesh=$2

# temporary mesh for roundtripping
tmp_output_mesh=$TEST_TMPDIR/mesh.stl

# call binary to roundtrip mesh
$roundtrip_stl_binary $test_mesh $tmp_output_mesh

# sanity check that written mesh *file* is the same as original
diff -q $test_mesh $tmp_output_mesh

echo "Mesh roundtrip finished with no errors."
