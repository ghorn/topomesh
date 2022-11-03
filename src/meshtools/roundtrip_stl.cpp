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

std::ostream& operator << (std::ostream &out, const glm::vec3 &v) {
  out << "{" << v.x << ", " << v.y << ", " << v.z << "}";
  return out;
}
std::ostream& operator << (std::ostream &out, const glm::ivec3 &v) {
  out << "{" << v.x << ", " << v.y << ", " << v.z << "}";
  return out;
}


// Usage: ./trim_bottom inputpath outputpath
int32_t main(int32_t argc, char *argv[]) {
  // Parse flags.
  if (argc != 3) {
    fprintf(stderr, "Need 2 command line arguments, input and output\n");
    exit(1);
  }
  std::string input_path = argv[1];
  std::string output_path = argv[2];
  assert(input_path.size() != 0);
  assert(output_path.size() != 0);

  // Read inputs.
  std::vector<glm::vec3> vertices;
  std::vector<glm::ivec3> triangles;
  ReadBinarySTL(input_path, vertices, triangles);
  std::cerr << "Loaded " << vertices.size() << " vertices and " << triangles.size() << " triangles from file." << std::endl;

  // Write outputs.
  WriteBinaryStl(output_path, vertices, triangles);

  // read inputs again and see if they're the same
  std::vector<glm::vec3> new_vertices;
  std::vector<glm::ivec3> new_triangles;
  ReadBinarySTL(input_path, new_vertices, new_triangles);

  bool fail = false;
  if (vertices.size() == new_vertices.size()) {
    for (size_t k=0; k<vertices.size(); k++) {
      glm::vec3 v0 = vertices.at(k);
      glm::vec3 v1 = new_vertices.at(k);
      if (v0 != v1) {
        std::cerr << "Vertex " << k << " changed from " << v0 << " to " << v1 << std::endl;
        fail = true;
      }
    }
  }
  if (triangles.size() == new_triangles.size()) {
    for (size_t k=0; k<triangles.size(); k++) {
      glm::ivec3 v0 = triangles.at(k);
      glm::ivec3 v1 = new_triangles.at(k);
      if (v0 != v1) {
        std::cerr << "Triangle index " << k << " changed from " << v0 << " to " << v1 << std::endl;
        fail = true;
      }
    }
  }
  if (vertices.size() != new_vertices.size()) {
    std::cerr << "Vertices length " << vertices.size() << " changed to " << new_vertices.size() << std::endl;
    fail = true;
  }
  if (triangles.size() != new_triangles.size()) {
    std::cerr << "Triangles length " << triangles.size() << " changed to " << new_triangles.size() << std::endl;
    fail = true;
  }
  if (fail) {
    std::exit(1);
  }
}
