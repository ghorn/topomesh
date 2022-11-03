#include <string>
#include <getopt.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ostream>
#include <vector>
#include <glm/glm.hpp>

#include "src/meshtools/stl.hpp"

// Usage: ./trim_bottom inputpath outputpath
int32_t main(int32_t argc, char *argv[]) {
  // Parse flags.
  if (argc != 4) {
    fprintf(stderr, "Need 3 command line arguments: input, output, scale.\n");
    exit(1);
  }
  const std::string input_path = argv[1];
  const std::string output_path = argv[2];
  const float scale_factor = static_cast<float>(atof(argv[3]));
  assert(input_path.size() != 0);
  assert(output_path.size() != 0);

  // Read inputs.
  std::vector<glm::vec3> vertices;
  std::vector<glm::ivec3> triangles;
  ReadBinarySTL(input_path, vertices, triangles);
  std::cerr << "Loaded " << vertices.size() << " vertices and " << triangles.size() << " triangles from file." << std::endl;

  // Translate vertices.
  for (glm::vec3 &vertex : vertices) {
    vertex.x *= scale_factor;
    vertex.y *= scale_factor;
    vertex.z *= scale_factor;
  }

  // Write outputs.
  WriteBinaryStl(output_path, vertices, triangles);
}
