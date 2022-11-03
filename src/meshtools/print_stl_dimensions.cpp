#include <string>
#include <getopt.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#include "src/meshtools/stl.hpp"

// Usage: ./trim_bottom inputpath outputpath
int32_t main(int32_t argc, char *argv[]) {
  // Parse flags.
  if (argc != 2) {
    fprintf(stderr, "Need 1 command line argument: input file\n");
    std::exit(1);
  }
  std::string input_path = argv[1];
  assert(input_path.size() != 0);

  // Read inputs.
  std::vector<glm::vec3> vertices;
  std::vector<glm::ivec3> triangles;
  ReadBinarySTL(input_path, vertices, triangles);

  if (vertices.size() == 0) {
    std::cout << "No vertices in this mesh." << std::endl;
    std::exit(1);
  }

  // Translate vertices.
  double min_x = vertices.at(0).x;
  double max_x = vertices.at(0).x;
  double min_y = vertices.at(0).y;
  double max_y = vertices.at(0).y;
  double min_z = vertices.at(0).z;
  double max_z = vertices.at(0).z;
  for (const glm::vec3 &vertex : vertices) {
    min_x = fmin(min_x, vertex.x);
    max_x = fmax(max_x, vertex.x);

    min_y = fmin(min_y, vertex.y);
    max_y = fmax(max_y, vertex.y);

    min_z = fmin(min_z, vertex.z);
    max_z = fmax(max_z, vertex.z);
  }

  std::cout << "X size: " << (max_x - min_x) << std::endl;
  std::cout << "Y size: " << (max_y - min_y) << std::endl;
  std::cout << "Z size: " << (max_z - min_z) << std::endl;
}
