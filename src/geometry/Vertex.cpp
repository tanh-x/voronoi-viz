#include "geometry/Vertex.hpp"

double Vertex::x() const {
    if (this->pos.isInfinite) exit(1);
    return this->pos.x;
}

double Vertex::y() const {
    if (this->pos.isInfinite) exit(1);
    return this->pos.y;
}

std::string Vertex::toString() const {
    return "v" + std::to_string(id);
}
