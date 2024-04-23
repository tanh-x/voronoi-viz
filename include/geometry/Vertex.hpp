#ifndef VORONOI_VIZ_VERTEX_HPP
#define VORONOI_VIZ_VERTEX_HPP

#include "utils/math/Vec2.hpp"
#include "HalfEdge.hpp"

class HalfEdge;

class Vertex {
public:
    int id;
    Vec2 pos;
    HalfEdge* incidentEdge;

    [[nodiscard]] double x() const;

    [[nodiscard]] double y() const;

    Vertex(int id, Vec2 v) :
        id(id),
        pos(v),
        incidentEdge(nullptr) {};
};

#endif //VORONOI_VIZ_VERTEX_HPP
