#ifndef VORONOI_VIZ_VERTEX_HPP
#define VORONOI_VIZ_VERTEX_HPP

#include <string>
#include "utils/math/Vec2.hpp"
#include "HalfEdge.hpp"

class HalfEdge;

class Vertex {
public:
    int id;
    Vec2 pos;
    HalfEdge* incidentEdge;
    bool isBoundary = false;

    [[nodiscard]] double x() const;

    [[nodiscard]] double y() const;

    Vertex(int id, Vec2 v) :
        id(id),
        pos(v),
        incidentEdge(nullptr) {};

    explicit Vertex(Vec2 v) :
        id(-1),
        pos(v),
        incidentEdge(nullptr) {};

    [[nodiscard]] std::string toString() const;

    static Vertex* boundary(Vec2 v);
};

#endif //VORONOI_VIZ_VERTEX_HPP
