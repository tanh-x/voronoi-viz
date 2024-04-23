#include "geometry/DCEL.hpp"

Vertex* DCEL::insertVertex(int id, Vec2 position) {
    auto* v = new Vertex(id, position);
    vertices.insert(v);
    return v;
}

HalfEdge* DCEL::insertEdge(Vertex* v1, Vertex* v2) {
    return nullptr;
}
