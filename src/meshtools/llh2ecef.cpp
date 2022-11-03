#include <cmath>
#include <string>
#include <getopt.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

#include "src/meshtools/stl.hpp"
// WGS84 equitorial radius.
constexpr double wgs84_A = 6378137.0;

// WGS84 flattening term.
constexpr double wgs84_F = 1 / 298.257223563;

// EGS84 eccentricity.
constexpr double wgs84_E = sqrt (2 * wgs84_F - wgs84_F * wgs84_F);

void llh2ecef(glm::vec3 &point) {
  constexpr double pi = M_PI;

  constexpr double origin_lon = -134.000833333333333;
  constexpr double lon_size = 0.016666746949261;

//  constexpr double origin_lat = 72.000416666666666;
//  constexpr double lat_size = -0.016666666666667;
  constexpr double origin_lat = 24.0004167;
  constexpr double lat_size = 0.016666666666667;
  constexpr double fudge_height = 1e3;
  
  const double lat = pi / 180. * (origin_lat + lat_size * static_cast<double>(point.y));
  const double lon = pi / 180. * (origin_lon + lon_size * static_cast<double>(point.x));
  const double height = static_cast<double>(point.z)*fudge_height;

  const double d = wgs84_E * sin(lat);
  const double n = wgs84_A / sqrt(1 - d * d);

  const double ecef0 = (n + height) * cos(lat) * cos(lon);
  const double ecef1 = (n + height) * cos(lat) * sin(lon);
  const double ecef2 = ((1 - wgs84_E * wgs84_E) * n + height) * sin(lat);

  point.x = static_cast<float>(ecef0 / wgs84_A);
  point.y = static_cast<float>(ecef1 / wgs84_A);
  point.z = static_cast<float>(ecef2 / wgs84_A);
}

// Usage: ./llh2ecef inputpath outputpath
int32_t main(int32_t argc, char *argv[]) {
  // Parse flags.
  if (argc != 3) {
    fprintf(stderr, "Need 2 command line arguments, input and output\n");
    exit(1);
  }
  const std::string input_path = argv[1];
  const std::string output_path = argv[2];
  assert(input_path.size() != 0);
  assert(output_path.size() != 0);

  std::vector<glm::vec3> points;
  std::vector<glm::ivec3> triangles;
  ReadBinarySTL(input_path, points, triangles);
  for (glm::vec3 &point : points) {
    llh2ecef(point);
  }
  WriteBinaryStl(output_path, points, triangles);
}
