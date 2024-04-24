#include <cstring>
#include <vector>
#include <fstream>
#include "tests.hpp"
#include "utils/math/Vec2.hpp"
#include "Fortune.hpp"


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


int main(int argc, char* argv[]) {
    runAllTests();

    [[maybe_unused]] bool animate = false;
    [[maybe_unused]] bool delaunay = false;
    [[maybe_unused]] float yStep = 1.0;
    std::vector<Vec2> sites;


    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--animate") == 0) {
            animate = true;
        } else if (strcmp(argv[i], "--delaunay") == 0) {
            delaunay = true;
        } else if (strcmp(argv[i], "--yStep") == 0) {
            if (i + 1 >= argc) continue;
            yStep = std::stof(argv[++i]);
        } else {
            // Assume it's a file path
            sites = parseSites(argv[i]);
            if (sites.empty()) {
                std::cerr << "ERROR: Provided file " << argv[i] << " has no parsed data" << std::endl;
                exit(1);
            }
        }
    }

    // Initialize the algorithm
    for (auto v: sites) {
        std::cout << v.toString() << std::endl;
    }

    // Start the algorithm
    DCEL* result = computeVoronoi(sites);
    printf("V: %d, HE: %d, F: %d", result->numVertices(), result->numHalfEdges(), result->numFaces());

    return 0;
}
