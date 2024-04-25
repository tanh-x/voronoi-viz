#ifndef VORONOI_VIZ_HALFEDGE_HPP
#define VORONOI_VIZ_HALFEDGE_HPP

#include <limits>
#include "Vertex.hpp"
#include "Face.hpp"
#include "utils/math/mathematics.hpp"

class Face;

class Vertex;

class HalfEdge {
public:
    Vertex* origin;
    Vertex* dest;
    HalfEdge* twin {nullptr};
    HalfEdge* next {nullptr};
    HalfEdge* prev {nullptr};
    Face* incidentFace {nullptr};

    HalfEdge() : origin {nullptr}, dest {nullptr} {}

    HalfEdge(Vertex* origin, Vertex* dest) : origin(origin), dest(dest) {}
};

struct VertexPair {
    Vertex* v1 = nullptr;
    Vertex* v2 = nullptr;

    double angle {QUIET_NAN};

    void offerVertex(Vertex* vertex);
};

#endif //VORONOI_VIZ_HALFEDGE_HPP
