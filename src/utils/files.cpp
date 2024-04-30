#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include "utils/files.hpp"

const char* readFile(const std::string &filePath) {
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    fileStream.close();
    std::string result = buffer.str();
    char* resultCStr = new char[result.length() + 1];
    std::strcpy(resultCStr, result.c_str());
    return resultCStr;
}

std::vector<Vec2> parseSites(const std::string &filepath) {
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot open provided input file: " << filepath << std::endl;
        exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    std::stringstream ss(content);

    char ignored;
    double x, y;

    std::vector<Vec2> sites;
    int id = 0;
    while (ss >> ignored >> x >> ignored >> y >> ignored) {
        sites.emplace_back(x, y, ++id);
    }

    return sites;
}
