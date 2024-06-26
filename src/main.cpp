#include <cstring>
#include <vector>
#include <fstream>
#include "tests.hpp"
#include "utils/math/Vec2.hpp"
#include "fortune/Fortune.hpp"
#include "utils/files.hpp"
#include "graphics/Renderer.hpp"

int main(int argc, char* argv[]) {
//    runAllTests();

    [[maybe_unused]] bool animate = false;
    [[maybe_unused]] bool delaunay = false;
    [[maybe_unused]] bool voronoi = false;
    std::vector<Vec2> sites;


    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--animate") == 0) {
            animate = true;
        } else if (strcmp(argv[i], "--delaunay") == 0) {
            delaunay = true;
        } else if (strcmp(argv[i], "--voronoi") == 0) {
            voronoi = true;
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
    FortuneSweeper algo(sites);
    DCEL* dcel = algo.computeAll();

    printf("\n\n--- FINISHED ---\n\n");
    printf("V: %d, HE: %d, F: %d\n", dcel->numVertices(), dcel->numHalfEdges(), dcel->numFaces());

    std::cout << "\nVertices:\n" << std::endl;
    for (auto v: dcel->vertices) {
        std::cout << v->toString() << " " << v->pos.toString() << std::endl;
    }

    std::cout << "\nEdges:\n" << std::endl;

    for (auto e: dcel->halfEdges) {
        Vertex* origin = e->origin;
        Vertex* dest = e->dest;
        std::cout << e->toString() << " - "
                  << (origin == nullptr ? "infty" : origin->toString() + origin->pos.toString()) << "->"
                  << (dest == nullptr ? "infty" : dest->toString() + dest->pos.toString()) << std::endl;
    }


    // TODO: DEBUGGING ONLY -------------
    std::ofstream file;
    std::streambuf* oldCoutStreamBuf;
    if (animate) {
        file = std::ofstream("dump");
        oldCoutStreamBuf = std::cout.rdbuf();
        std::cout.rdbuf(file.rdbuf());
    } else {
        std::cout << "\n\nDEBUGGING: Generated python mpl syntax:\n```\n" << std::endl;
    }
    std::cout << "sites = np.array([" << std::endl;
    for (auto s: sites) {
        std::cout << "\t(" << std::to_string(s.x) << ", " << std::to_string(s.y) << ")," << std::endl;
    }
    std::cout << "])\n\n# Vertex list\nverts = np.array([" << std::endl;
    for (auto v: dcel->vertices) {
        std::cout << '\t' << v->pos.toString() << "," << (v->isBoundary ? "\t# boundary" : "") << std::endl;
    }
    std::cout << "])\n\n# Edge list\nv1 = np.array([" << std::endl;
    for (auto e: dcel->halfEdges) {
        std::cout << '\t';
        Vertex* origin = e->origin;
        if (origin == nullptr || e->dest == nullptr) std::cout << "# ";
        std::cout << (origin == nullptr ? "(0, 0)" : origin->pos.toString()) << "," << std::endl;
    }
    std::cout << "])\n\nv2 = np.array([" << std::endl;
    for (auto e: dcel->halfEdges) {
        std::cout << '\t';
        Vertex* dest = e->dest;
        if (dest == nullptr || e->origin == nullptr) std::cout << "# ";
        std::cout << (dest == nullptr ? "(0, 0)" : dest->pos.toString()) << "," << std::endl;
    }
    std::cout << "])\n\n" << std::endl;
    if (animate) {
        std::cout.rdbuf(oldCoutStreamBuf);  // Restore old std::cout buffer
        file.close();
    }
    // ----------------------

    printf("\n****** Voronoi diagram ******\n");
    dcel->printOutputVoronoiStyle();

    DCEL* delaunayTriangulation = algo.factory->buildDualGraph();

    printf("\n****** Delaunay triangulation ******\n");
    delaunayTriangulation->printOutputDelaunayStyle();

    printf("\n\n");

    if (animate) return 0;

    // Set up renderer
    Renderer renderer = Renderer(720, 720);

    if ((!voronoi && !delaunay) || voronoi) {
        renderer.initVertexObjects(dcel);
        renderer.initSiteVertexObjects(sites);
        renderer.startRender();  // Blocking call
        renderer.terminate();
    } else if (delaunay) {
        renderer.initVertexObjects(delaunayTriangulation);
        renderer.initSiteVertexObjects(sites);
        renderer.startRender();  // Blocking call
        renderer.terminate();
    }
    return 0;
}