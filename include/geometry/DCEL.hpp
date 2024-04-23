
#ifndef VORONOI_VIZ_DCEL_HPP
#define VORONOI_VIZ_DCEL_HPP

#include <unordered_set>
#include "Vertex.hpp"
#include "Face.hpp"

class DCEL {
public:
    std::unordered_set<Vertex*> vertices;
    std::unordered_set<HalfEdge*> edges;
    std::unordered_set<Face*> faces;

    ~DCEL() {
        for (auto v: vertices) delete v;
        for (auto e: edges) delete e;
        for (auto f: faces) delete f;
    }

    Vertex* insertVertex(int id, Vec2 position);

    HalfEdge* insertEdge(Vertex* v1, Vertex* v2);
};

#endif //VORONOI_VIZ_DCEL_HPP
