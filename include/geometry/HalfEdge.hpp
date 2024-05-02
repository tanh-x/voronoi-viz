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
    double angle;
    HalfEdge* twin {nullptr};
    HalfEdge* next {nullptr};
    HalfEdge* prev {nullptr};
    Face* incidentFace {nullptr};
    bool unbounded = false;

    HalfEdge() : origin {nullptr}, dest {nullptr}, angle(QUIET_NAN) {}

    HalfEdge(Vertex* origin, Vertex* dest);

    [[nodiscard]] const char* toString() const;

    [[nodiscard]] HalfEdge* generateTwin() const;

    void bindTwins(HalfEdge* other);

    void chainNext(HalfEdge* other);

    void chainPrev(HalfEdge* other);
};

struct HalfEdgeAngleComparator {
    bool operator()(const HalfEdge* lhs, const HalfEdge* rhs) const {
        return lhs->angle < rhs->angle;
    }
};

struct VertexPair {
    Vertex* v1 = nullptr;
    Vertex* v2 = nullptr;
    Vec2* incidentSiteA;
    Vec2* incidentSiteB;

    double angle {QUIET_NAN};

    void offerVertex(Vertex* vertex);
};

#endif //VORONOI_VIZ_HALFEDGE_HPP
