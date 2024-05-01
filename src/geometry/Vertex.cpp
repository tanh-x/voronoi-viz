#include "geometry/Vertex.hpp"

double Vertex::x() const {
    return this->pos.x;
}

double Vertex::y() const {
    return this->pos.y;
}

std::string Vertex::toString() const {
    return "v" + std::to_string(id);
}

Vertex* Vertex::boundary(Vec2 v) {
    Vertex* result = new Vertex(v);
    result->isBoundary = true;
    return result;
}
