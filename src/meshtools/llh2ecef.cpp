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


static glm::dmat3 DcmEcef2Ned(const double lat_rad, const double lon_rad) {
  double sin_lat = sin(lat_rad);
  double cos_lat = cos(lat_rad);
  double sin_lon = sin(lon_rad);
  double cos_lon = cos(lon_rad);
  glm::dmat3 dcm;
  dcm[0][0] = -sin_lat * cos_lon;
  dcm[1][0] = -sin_lat * sin_lon;
  dcm[2][0] = cos_lat;
  dcm[0][1] = -sin_lon;
  dcm[1][1] = cos_lon;
  dcm[2][1] = 0.0;
  dcm[0][2] = -cos_lat * cos_lon;
  dcm[1][2] = -cos_lat * sin_lon;
  dcm[2][2] = -sin_lat;
  return dcm;
}

static glm::dvec3 Llh2Ecef(const double lat, const double lon, const double height) {
  const double d = wgs84_E * sin(lat);
  const double n = wgs84_A / sqrt(1 - d * d);

  glm::dvec3 ecef;
  ecef[0] = (n + height) * cos(lat) * cos(lon);
  ecef[1] = (n + height) * cos(lat) * sin(lon);
  ecef[2] = ((1 - wgs84_E * wgs84_E) * n + height) * sin(lat);

  return ecef;
}



int32_t main(int32_t argc, char *argv[]) {
  // Parse flags.
  if (argc != 13) {
    fprintf(stderr, "Usage: ./llh2ecef inputpath outputpath lon0 dlon_dpixel lat0 dlat_dpixel n_lat n_lon min_height max_height target_size z_exag\n");
    exit(1);
  }
  const std::string input_path = argv[1];
  const std::string output_path = argv[2];
  assert(input_path.size() != 0);
  assert(output_path.size() != 0);
  const double lon0_deg = std::stod(argv[3]);
  const double dlon_deg_dpixel = std::stod(argv[4]);
  const double lat0_deg = std::stod(argv[5]);
  const double dlat_deg_dpixel = std::stod(argv[6]);
  const int32_t n_lon = std::stoi(argv[7]);
  const int32_t n_lat = std::stoi(argv[8]);
  const double min_height = std::stod(argv[9]);
  const double max_height = std::stod(argv[10]);
  const double target_size = std::stod(argv[11]);
  const double z_exag = std::stod(argv[12]);

  // print all the command line arguments
  fprintf(stderr, "     input_path:  %s\n" , input_path.c_str());
  fprintf(stderr, "    output_path:  %s\n",  output_path.c_str());
  fprintf(stderr, "       lon0_deg:  %.15f\n", lon0_deg);
  fprintf(stderr, "dlon_deg_dpixel:  %.15f\n",  dlon_deg_dpixel);
  fprintf(stderr, "       lat0_deg:  %.15f\n",  lat0_deg);
  fprintf(stderr, "dlat_deg_dpixel:  %.15f\n",  dlat_deg_dpixel);
  fprintf(stderr, "          n_lon:  %d\n",  n_lon);
  fprintf(stderr, "          n_lat:  %d\n",  n_lat);
  fprintf(stderr, "     min_height:  %.2f\n", min_height);
  fprintf(stderr, "     max_height:  %.2f\n", max_height);
  fprintf(stderr, "   target_size:  %.2f\n", target_size);
  fprintf(stderr, "        z_exag:  %.2f\n", z_exag);

  // Load the input mesh.
  std::vector<glm::vec3> points;
  std::vector<glm::ivec3> triangles;
  ReadBinarySTL(input_path, points, triangles);

  // Reference ECEF
  const double center_lat_deg = lat0_deg + 0.5 * dlat_deg_dpixel * static_cast<double>(n_lat);
  const double center_lon_deg = lon0_deg + 0.5 * dlon_deg_dpixel * static_cast<double>(n_lon);
  const glm::dvec3 ref_ecef = Llh2Ecef(center_lat_deg * M_PI / 180.,
                                       center_lon_deg * M_PI / 180.,
                                       0.);

  const glm::dmat3 dcm_ecef2ned = DcmEcef2Ned(center_lat_deg * M_PI / 180.,
                                              center_lon_deg * M_PI / 180.);

  // glm::mat3 dcm_ecef2ned = Ecef2NedMatrix(ref_ecef);
  double max_x = -std::numeric_limits<double>::infinity();
  double min_x = std::numeric_limits<double>::infinity();
  double max_y = -std::numeric_limits<double>::infinity();
  double min_y = std::numeric_limits<double>::infinity();
  double max_z = -std::numeric_limits<double>::infinity();
  double min_z = std::numeric_limits<double>::infinity();

  double min_lon = std::numeric_limits<double>::infinity();
  double max_lon = -std::numeric_limits<double>::infinity();
  double min_lat = std::numeric_limits<double>::infinity();
  double max_lat = -std::numeric_limits<double>::infinity();

  const double lon0 = lon0_deg * M_PI / 180.;
  const double lat0 = lat0_deg * M_PI / 180.;
  const double dlon_dpixel = dlon_deg_dpixel * M_PI / 180.;
  const double dlat_dpixel = dlat_deg_dpixel * M_PI / 180.;

  const double latF = lat0 + dlat_dpixel * static_cast<double>(n_lat);

  for (glm::vec3 &point : points) {
    const double point_x = static_cast<double>(point.x);
    const double point_y = static_cast<double>(point.y);
    const double point_z = z_exag*(min_height + (max_height - min_height) * static_cast<double>(point.z));

    const double lat = latF - dlat_dpixel * point_y;
    const double lon = lon0 + dlon_dpixel * point_x;

    const double height = point_z;
    const glm::dvec3 point_ecef = Llh2Ecef(lat, lon, height);
    point = dcm_ecef2ned * (point_ecef - ref_ecef);

    // compute min/max of x/y/z
    max_x = std::max(max_x, static_cast<double>(point.x));
    min_x = std::min(min_x, static_cast<double>(point.x));
    max_y = std::max(max_y, static_cast<double>(point.y));
    min_y = std::min(min_y, static_cast<double>(point.y));
    max_z = std::max(max_z, point_z);
    min_z = std::min(min_z, point_z);

    // compute min/max of lon/lat
    min_lon = std::min(min_lon, lon);
    max_lon = std::max(max_lon, lon);
    min_lat = std::min(min_lat, lat);
    max_lat = std::max(max_lat, lat);
  }

  fprintf(stderr, "min_x: %.2f\n", min_x);
  fprintf(stderr, "max_x: %.2f\n", max_x);
  fprintf(stderr, "min_y: %.2f\n", min_y);
  fprintf(stderr, "max_y: %.2f\n", max_y);
  fprintf(stderr, "min_z: %.2f\n", min_z);
  fprintf(stderr, "max_z: %.2f\n", max_z);

  fprintf(stderr, "min_lon: %.6f [deg]\n", (min_lon * 180.0 / M_PI));
  fprintf(stderr, "max_lon: %.6f [deg]\n", (max_lon * 180.0 / M_PI));
  fprintf(stderr, "min_lat: %.6f [deg]\n", (min_lat * 180.0 / M_PI));
  fprintf(stderr, "max_lat: %.6f [deg]\n", (max_lat * 180.0 / M_PI));

  // output translation/scaling
  const float xy_scale_factor = static_cast<float>(target_size / std::min(max_x - min_x, max_y - min_y));
  for (glm::vec3 &point : points) {
    point.z -= static_cast<float>(min_z);
    point *= xy_scale_factor;
  }

  WriteBinaryStl(output_path, points, triangles);
  fprintf(stderr, "wrote mesh to %s\n", output_path.c_str());
}
