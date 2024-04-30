#ifndef VORONOI_VIZ_FILES_HPP
#define VORONOI_VIZ_FILES_HPP

#include <string>
#include <vector>
#include "utils/math/Vec2.hpp"

const char* readFile(const std::string &filePath);

std::vector<Vec2> parseSites(const std::string &filepath);


#endif //VORONOI_VIZ_FILES_HPP
