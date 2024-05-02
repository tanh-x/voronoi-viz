#include "geometry/Vertex.hpp"

double Vertex::x() const {
    return this->pos.x;
}

double Vertex::y() const {
    return this->pos.y;
}

std::string Vertex::toString() const {
    return (isBoundary ? 'b' : 'v') + std::to_string(label);
}

Vertex* Vertex::boundary(Vec2 v, int label) {
    auto* result = new Vertex(label, v);
    result->isBoundary = true;
    return result;
}
