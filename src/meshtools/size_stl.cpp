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

int32_t main(int32_t argc, char *argv[]) {
  // Parse flags.
  if (argc != 9) {
    fprintf(stderr, "Usage: size_stl input output dmeter_dpixel_x dmeter_dpixel_y min_height max_height target_size z_exag.\n");
    exit(1);
  }
  const std::string input_path = argv[1];
  const std::string output_path = argv[2];
  const float dmeter_dpixel_x = std::stof(argv[3]);
  const float dmeter_dpixel_y = std::stof(argv[4]);
  const float min_height = std::stof(argv[5]);
  const float max_height = std::stof(argv[6]);
  const float target_size = std::stof(argv[7]);
  const float z_exag = std::stof(argv[8]);

  assert(input_path.size() != 0);
  assert(output_path.size() != 0);

  // Read inputs.
  std::vector<glm::vec3> vertices;
  std::vector<glm::ivec3> triangles;
  ReadBinarySTL(input_path, vertices, triangles);
  std::cerr << "Loaded " << vertices.size() << " vertices and " << triangles.size() << " triangles from file." << std::endl;

  if (vertices.size() == 0) {
    std::cerr << "No vertices in this mesh." << std::endl;
    std::exit(1);
  }

  // Compute min and max coordinates.
  float min_x = vertices.at(0).x;
  float max_x = vertices.at(0).x;
  float min_y = vertices.at(0).y;
  float max_y = vertices.at(0).y;
  float min_z = vertices.at(0).z;
  for (const glm::vec3 &vertex : vertices) {
    min_x = std::fmin(min_x, vertex.x);
    max_x = std::fmax(max_x, vertex.x);

    min_y = std::fmin(min_y, vertex.y);
    max_y = std::fmax(max_y, vertex.y);

    min_z = std::fmin(min_z, vertex.z);
  }
  float center_x = 0.5f * (min_x + max_x);
  float center_y = 0.5f * (min_y + max_y);

  // We need to apply a few scalings
  // 1. x and y could be in units of multiple meters (like 2x meters)
  // 2. z is from 0 to 1, but needs to be from min_height to max_height
  // 3. We want whichever of smaller of x and y to be set to target_size.
  //    The shortest size determines the size, because wood comes in long boards and the
  //    longest size is assumed to fit.
  // 4. also scale z by z_exag
  if (std::abs(dmeter_dpixel_x) != std::abs(dmeter_dpixel_y)) {
    // TODO(greg): figure out which is x and which is y
    std::cerr << "Warning: dmeter_dpixel_x and dmeter_dpixel_y are not equal. This will cause the mesh to be stretched." << std::endl;
    std::exit(1);
  }


  const float target_scale_factor = target_size / std::fmin(std::abs(dmeter_dpixel_x) * (max_x - min_x), std::abs(dmeter_dpixel_y)*(max_y - min_y));

  const float x_scale_factor = target_scale_factor * std::abs(dmeter_dpixel_x);
  const float y_scale_factor = target_scale_factor * std::abs(dmeter_dpixel_y);
  const float z_scale_factor = target_scale_factor * (max_height - min_height) * z_exag;

  // Translate vertices.
  for (glm::vec3 &vertex : vertices) {
    vertex.x = x_scale_factor * (vertex.x - center_x);
    vertex.y = y_scale_factor * (vertex.y - center_y);
    vertex.z = z_scale_factor * (vertex.z - min_z);
  }

  // Write outputs.
  WriteBinaryStl(output_path, vertices, triangles);
}
