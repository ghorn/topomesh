#pragma once

#include <functional>
#include <glm/glm.hpp>

template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
  template <>
  struct hash<glm::vec3>
  {
    std::size_t operator()(const glm::vec3& vertex) const
    {
      std::size_t seed = 0;
      hash_combine(seed, vertex.x);
      hash_combine(seed, vertex.y);
      hash_combine(seed, vertex.z);
      return seed;
    }
  };
}

namespace std {
  template <>
  struct hash<glm::vec2>
  {
    std::size_t operator()(const glm::vec2& vertex) const
    {
      std::size_t seed = 0;
      hash_combine(seed, vertex.x);
      hash_combine(seed, vertex.y);
      return seed;
    }
  };
}
