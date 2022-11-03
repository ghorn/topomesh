#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

void WriteBinaryStl(
    const std::string &path,
    const std::vector<glm::vec3> &points,
    const std::vector<glm::ivec3> &triangles);

void ReadBinarySTL(
    const std::string &path,
    std::vector<glm::vec3> &points,
    std::vector<glm::ivec3> &triangles);
