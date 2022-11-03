#include "ply.hpp"

#include <cstdlib>
#include <cstdio>

void WritePlyHeader(FILE * const output, const uint32_t vertex_count, const uint32_t triangle_count) {
  fprintf(output, "ply\r\n");
  fprintf(output, "format binary_little_endian 1.0\r\n");
  fprintf(output, "element vertex %u\r\n", vertex_count);
  fprintf(output, "property float x\r\n");
  fprintf(output, "property float y\r\n");
  fprintf(output, "property float z\r\n");
  fprintf(output, "element face %u\r\n", triangle_count);
  fprintf(output, "property list uchar uint vertex_indices\r\n");
  fprintf(output, "end_header\r\n");
}

void ReadPlyHeader(FILE * const input, uint32_t *vertex_count, uint32_t *triangle_count) {
  char buffer[81];
  int lol = 0;

  //fprintf(output, "ply\r\n");
  lol = fscanf(input, "%80s\n", buffer);

  //fprintf(output, "format binary_little_endian 1.0\r\n");
  lol = fscanf(input, "%80s %80s %80s\n", buffer, buffer, buffer);

  //fprintf(output, "element vertex %u\r\n", vertex_count);
  lol = fscanf(input, "%s %s %u\n", buffer, buffer, vertex_count);

  //fprintf(output, "property float x\r\n");
  //fprintf(output, "property float y\r\n");
  //fprintf(output, "property float z\r\n");
  for (int k=0; k<3; k++) {
    lol = fscanf(input, "%s %s %s\n", buffer, buffer, buffer);
  }

  //fprintf(output, "element face %u\r\n", triangle_count);
  lol = fscanf(input, "%80s %80s %u\n", buffer, buffer, triangle_count);

  //fprintf(output, "property list uchar uint vertex_indices\r\n");
  lol = fscanf(input, "%80s %80s %80s %80s %80s\n", buffer, buffer, buffer, buffer, buffer);

  //fprintf(output, "end_header\r\n");
  lol = fscanf(input, "%80s\n", buffer);

  (void)lol;
}

void WriteTriangleHeader(FILE * const output) {
  // Write triangle header.
  const uint8_t three = 3; // This triangle will have three vertices, big surprise.
  if (fwrite(&three, 1, 1, output) != 1) {
    fprintf(stderr, "Error writing 'three' as uint8_t\n");
    std::exit(1);
  }
}

void ReadTriangleHeader(FILE * const input) {
  // Write triangle header.
  uint8_t three = 0; // This triangle will have three vertices, big surprise.
  if (fread(&three, 1, 1, input) != 1) {
    fprintf(stderr, "Error reading 'three' as uint8_t\n");
    std::exit(1);
  }
  if (three != 3) {
    fprintf(stderr, "Error reading 'three' in triangle header. It is not 3 it is %d\n", (int32_t)three);
    std::exit(1);
  }
}

void WriteVertex(FILE * const output, const glm::vec3 &vertex) {
  static_assert(sizeof(glm::vec3) == 3*sizeof(float));
  // Write this new vertex to file.
  if (fwrite(&vertex, 4, 3, output) != 3) {
    fprintf(stderr, "Error writing vertex\n");
    std::exit(1);
  }
}

void ReadVertex(FILE * const input, glm::vec3 *vertex) {
  static_assert(sizeof(glm::vec3) == 3*sizeof(float));
  if (fread(vertex, 4, 3, input) != 3) {
    fprintf(stderr, "Error reading vertex\n");
    std::exit(1);
  }
}


void WriteVertexIndex(FILE * const output, const uint32_t vertex_index) {
  static_assert(sizeof(vertex_index) == 4);
  // write the vertex index to the triangle file
  if (fwrite(&vertex_index, 4, 1, output) != 1) {
    fprintf(stderr, "Error writing vertex indexu\n");
    std::exit(1);
  }
}

void ReadVertexIndex(FILE * const input, uint32_t *vertex_index) {
  static_assert(sizeof(*vertex_index) == 4);
  // write the vertex index to the triangle file
  if (fread(vertex_index, 4, 1, input) != 1) {
    fprintf(stderr, "Error reading vertex indexu\n");
    std::exit(1);
  }
}

void SavePly(const std::string &path,
             const std::vector<glm::vec3> &points,
             const std::vector<glm::ivec3> &triangles) {
  FILE *output = fopen(path.c_str(), "w");
  if (output == NULL) {
    fprintf(stderr, "Error opening output file %s.\n", path.c_str());
    exit(1);
  }

  WritePlyHeader(output, (uint32_t)points.size(), (uint32_t)triangles.size());

  // write vertex list
  for (const glm::vec3 & vertex : points) {
    WriteVertex(output, vertex);
  }

  // Write triangle list
  for (const glm::ivec3 & triangle : triangles) {
    WriteTriangleHeader(output);
    WriteVertexIndex(output, static_cast<uint32_t>(triangle[0]));
    WriteVertexIndex(output, static_cast<uint32_t>(triangle[1]));
    WriteVertexIndex(output, static_cast<uint32_t>(triangle[2]));
  }
}

void LoadPly(const std::string &path,
             std::vector<glm::vec3> *points,
             std::vector<glm::ivec3> *triangles) {
  FILE *input = fopen(path.c_str(), "r");
  if (input == NULL) {
    fprintf(stderr, "Error opening input\n");
    std::exit(1);
  }

  uint32_t vertex_count = 0;
  uint32_t triangle_count = 0;

  ReadPlyHeader(input, &vertex_count, &triangle_count);
  fprintf(stderr, "Reading %u vertices, %u faces\n", vertex_count, triangle_count);

  // Read vertices
  points->clear();
  points->reserve(vertex_count);
  for (uint32_t k=0; k<vertex_count; k++) {
    static_assert(sizeof(glm::vec3) == 3*sizeof(float));
    glm::vec3 vertex;
    ReadVertex(input, &vertex);
    points->emplace_back(vertex);
  }

  // Read triangles
  triangles->clear();
  triangles->reserve(triangle_count);
  for (uint32_t k=0; k<triangle_count; k++) {
    ReadTriangleHeader(input);
    glm::ivec3 triangle;
    ReadVertexIndex(input, reinterpret_cast<uint32_t*>(&triangle[0]));
    ReadVertexIndex(input, reinterpret_cast<uint32_t*>(&triangle[1]));
    ReadVertexIndex(input, reinterpret_cast<uint32_t*>(&triangle[2]));
    triangles->emplace_back(triangle);
  }

  fclose(input);
}
