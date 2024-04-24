#include <stdexcept>
#include "geometry/HalfEdge.hpp"
#include "geometry/Vertex.hpp"

void VertexPair::offerVertex(Vertex* vertex) {
    if (v1 == nullptr) this->v1 = vertex;
    else if (v2 == nullptr) this->v2 = vertex;
    else throw std::invalid_argument("Vertex already has two vertices");
}
