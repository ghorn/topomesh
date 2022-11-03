#pragma once

#include <inttypes.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>

void WritePlyHeader(FILE * const output, const uint32_t vertex_count, const uint32_t triangle_count);
void WriteTriangleHeader(FILE * const output);
void WriteVertex(FILE * const output, const glm::vec3 &vertex);
void WriteVertexIndex(FILE * const output, const uint32_t vertex_index);
void SavePly(const std::string &path,
             const std::vector<glm::vec3> &points,
             const std::vector<glm::ivec3> &triangles);
void LoadPly(const std::string &path,
             std::vector<glm::vec3> *points,
             std::vector<glm::ivec3> *triangles);
