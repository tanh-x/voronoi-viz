#include <stdexcept>
#include <cmath>
#include <cassert>
#include "geometry/HalfEdge.hpp"
#include "geometry/Vertex.hpp"

void VertexPair::offerVertex(Vertex* vertex) {
    if (v1 == nullptr) this->v1 = vertex;
    else if (v2 == nullptr) this->v2 = vertex;
    else {
        // Third vertex is offered, check if we want to extend the segment
        Vec2 dir12 = v2->pos - v1->pos;
        Vec2 dir13 = vertex->pos - v1->pos;
        Vec2 dir23 = vertex->pos - v2->pos;

        // If cross product is near zero, the points are considered collinear
        if (std::abs(dir12.cross(dir13)) > NUMERICAL_TOLERANCE) {
            throw std::invalid_argument("Third vertex offered, but is not collinear");
        }
        assert(std::abs(dir12.cross(dir23)) < NUMERICAL_TOLERANCE);
        assert(std::abs(dir23.cross(dir13)) < NUMERICAL_TOLERANCE);

        double dist12 = dir12.norm();
        double dist13 = dir13.norm();
        double dist23 = dir23.norm();

        // Third vertex essentially same as one of the vertices
        if (dist13 < NUMERICAL_TOLERANCE || dist23 < NUMERICAL_TOLERANCE) return;

        if (dist13 > dist12 && dist13 > dist23) {
            // 1-2-3
            assert(
                (v1->x() < v2->x() && v2->x() < vertex->pos.x)
                || (v1->x() > v2->x() && v2->x() > vertex->pos.x)
            );
            this->v2 = vertex;
        } else if (dist23 > dist12 && dist23 > dist13) {
            // 2-1-3
            assert(
                (v2->x() < v1->x() && v1->x() < vertex->pos.x)
                || (v2->x() > v1->x() && v1->x() > vertex->pos.x)
            );
            this->v1 = vertex;
        }

    }
}

const char* HalfEdge::toString() const {
    char* result = new char[64];
    sprintf(
        result, "%s%d,%s%d",
        (origin->isBoundary ? "b" : ""), origin->label,
        (dest->isBoundary ? "b" : ""), dest->label
    );

    return result;
}

HalfEdge* HalfEdge::generateTwin() const {
    auto* result = new HalfEdge(dest, origin);  // NOLINT
    return result;
}

void HalfEdge::bindTwins(HalfEdge* other) {
    this->twin = other;
    other->twin = this;
}

void HalfEdge::chainNext(HalfEdge* other) {
    this->next = other;
    other->prev = this;
}

void HalfEdge::chainPrev(HalfEdge* other) {
    this->prev = other;
    other->next = this;
}

HalfEdge::HalfEdge(Vertex* origin, Vertex* dest) : origin(origin), dest(dest) {
    if (origin != nullptr && dest != nullptr) angle = atan2(dest->y() - origin->y(), dest->x() - origin->x());
    else angle = QUIET_NAN;
}
