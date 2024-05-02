#include "geometry/Face.hpp"

std::string Face::toString() const {
    return "f" + std::to_string(label);
}

void Face::offerComponent(HalfEdge* edge) {
    if (edge->unbounded) {
        unbounded = true;
        outer = nullptr;
        inner = edge;
    } else if (!unbounded) {
        outer = edge;
    } else {
        inner = edge;
    }
}
