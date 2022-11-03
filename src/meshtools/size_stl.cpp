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
    fprintf(stderr, "Need 3 command line arguments: input, output, target size.\n");
    exit(1);
  }
  const std::string input_path = argv[1];
  const std::string output_path = argv[2];
  const double target_size = atof(argv[3]);
  assert(input_path.size() != 0);
  assert(output_path.size() != 0);

  // Read inputs.
  std::vector<glm::vec3> vertices;
  std::vector<glm::ivec3> triangles;
  ReadBinarySTL(input_path, vertices, triangles);
  std::cerr << "Loaded " << vertices.size() << " vertices and " << triangles.size() << " triangles from file." << std::endl;

  if (vertices.size() == 0) {
    std::cout << "No vertices in this mesh." << std::endl;
    std::exit(0);
  }

  // Compute min and max coordinates.
  double min_x = vertices.at(0).x;
  double max_x = vertices.at(0).x;
  double min_y = vertices.at(0).y;
  double max_y = vertices.at(0).y;
  for (const glm::vec3 &vertex : vertices) {
    min_x = fmin(min_x, vertex.x);
    max_x = fmax(max_x, vertex.x);

    min_y = fmin(min_y, vertex.y);
    max_y = fmax(max_y, vertex.y);
  }

  // The shortest size determines the size, because wood comes in long boards and the
  // longest size is assumed to fit.
  const float scale_factor = static_cast<float>(target_size / fmin(max_x - min_x, max_y - min_y));

  // Translate vertices.
  for (glm::vec3 &vertex : vertices) {
    vertex.x *= scale_factor;
    vertex.y *= scale_factor;
    vertex.z *= scale_factor;
  }

  // Write outputs.
  WriteBinaryStl(output_path, vertices, triangles);
}
