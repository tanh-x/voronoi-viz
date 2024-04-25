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
//    runAllTests();

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
    printf("V: %d, HE: %d, F: %d\n", result->numVertices(), result->numHalfEdges(), result->numFaces());

    std::cout << "Vertices:\n" << std::endl;
    for (auto v: result->vertices) {
        std::cout << v->toString() << v->pos.toString() << std::endl;
    }

    std::cout << "Edges:\n" << std::endl;

    for (auto e: result->halfEdges) {
        Vertex* origin = e->origin;
        Vertex* dest = e->dest;
        std::cout << (origin == nullptr ? "infty" : origin->toString() + origin->pos.toString()) << "->"
                  << (dest == nullptr ? "infty" : dest->toString() + dest->pos.toString()) << std::endl;
    }




    // TODO: DEBUGGING ONLY -------------
    std::cout << "DEBUGGING: Generated python mpl syntax:\n```" << std::endl;
    std::cout << "sites = np.array([" << std::endl;
    for (auto s: sites) {
        std::cout << "\t(" << std::to_string(s.x) << ", " << std::to_string(s.y) << ")," << std::endl;
    }
    std::cout << "])\n\n# Vertex list\nverts = np.array([" << std::endl;
    for (auto v: result->vertices) {
        std::cout << '\t' << v->pos.toString() << "," << std::endl;
    }
    std::cout << "])\n\n# Edge list\nv1 = np.array([" << std::endl;
    for (auto e: result->halfEdges) {
        std::cout << '\t';
        Vertex* origin = e->origin;
        if (origin == nullptr || e->dest == nullptr) std::cout << "# ";
        std::cout << (origin == nullptr ? "(0, 0)" : origin->pos.toString()) << "," << std::endl;
    }
    std::cout << "])\n\nv2 = np.array([" << std::endl;
    for (auto e: result->halfEdges) {
        std::cout << '\t';
        Vertex* dest = e->dest;
        if (dest == nullptr || e->origin == nullptr) std::cout << "# ";
        std::cout << (dest == nullptr ? "(0, 0)" : dest->pos.toString()) << "," << std::endl;
    }
    std::cout << "])" << std::endl;

    return 0;
}
