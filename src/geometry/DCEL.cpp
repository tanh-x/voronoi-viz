#include <algorithm>
#include <set>
#include <cassert>
#include <cmath>
#include "geometry/DCEL.hpp"
#include "utils/math/mathematics.hpp"

// Any reasonable number (around 0.1 to 0.5), aesthetics only
#define BOUNDING_BOX_PADDING 0.362160297

Vertex* DCEL::insertVertex(int id, Vec2 position) {
    auto* v = new Vertex(id, position);
//    v->pos.x = std::clamp(v->pos.x, bottomLeftBounds.x, topRightBounds.x);
//    v->pos.y = std::clamp(v->pos.y, bottomLeftBounds.y, topRightBounds.y);
    return insert(v);
}

HalfEdge* DCEL::insertEdge(Vertex* v1, Vertex* v2) {
    auto* e = new HalfEdge(v1, v2);
    halfEdges.push_back(e);
    return e;
}

int DCEL::numVertices() const {
    return static_cast<int>(vertices.size());
}

int DCEL::numHalfEdges() const {
    return static_cast<int>(halfEdges.size());
}

int DCEL::numEdges() const {
    return static_cast<int>(halfEdges.size() / 2);
}

int DCEL::numFaces() const {
    return static_cast<int>(faces.size());
}

double DCEL::getCenteredX(double x) const {
    return (x - centroid.x) / majorAxis;
//    return x / 10.0;
}

double DCEL::getCenteredY(double y) const {
    return (y - centroid.y) / majorAxis;
//    return y / 10.0;
}

Vertex* DCEL::insertVertex(Vec2 position) {
    return insertVertex(-1, position);
}

Vertex* DCEL::insert(Vertex* vertex) {
    vertices.push_back(vertex);
    return vertex;
}

struct VertexPairComparator {
    bool operator()(const VertexPair* lhs, const VertexPair* rhs) const {
        // Order-independent comparison of vertex pointers
        std::pair<Vertex*, Vertex*> minmax_lhs = std::minmax(lhs->v1, lhs->v2);
        std::pair<Vertex*, Vertex*> minmax_rhs = std::minmax(rhs->v1, rhs->v2);
        return minmax_lhs < minmax_rhs;
    }
};


DCEL* DCELFactory::createDCEL(const std::vector<Vec2> &sites) {
    // Initialize the DCEL
    auto* dcel = new DCEL();

    // Define the bounding box corner coordinates
    auto bottomLeft = Vec2(DOUBLE_INFINITY, DOUBLE_INFINITY);
    auto topRight = Vec2(-DOUBLE_INFINITY, -DOUBLE_INFINITY);

    // Adjust the boundary box corners
    for (auto &s: sites) {
        bottomLeft.x = std::min(bottomLeft.x, s.x);
        bottomLeft.y = std::min(bottomLeft.y, s.y);
        topRight.x = std::max(topRight.x, s.x);
        topRight.y = std::max(topRight.y, s.y);
    }

    for (auto &v: vertices) {
        dcel->vertices.push_back(v);

        // Continue adjusting bounding box corners
        bottomLeft.x = std::min(bottomLeft.x, v->x());
        bottomLeft.y = std::min(bottomLeft.y, v->y());
        topRight.x = std::max(topRight.x, v->x());
        topRight.y = std::max(topRight.y, v->y());
    }


    // Equalize axes
    double width = topRight.x - bottomLeft.x;
    double height = topRight.y - bottomLeft.y;
    double majorAxis = std::max(width, height);

    Vec2 centroid = {(topRight.x + bottomLeft.x) * 0.5, (topRight.y + bottomLeft.y) * 0.5};
    topRight = centroid + Vec2(majorAxis, majorAxis) * 0.5;
    bottomLeft = centroid - Vec2(majorAxis, majorAxis) * 0.5;

    // Add some padding
    Vec2 padding = Vec2(majorAxis, majorAxis) * BOUNDING_BOX_PADDING;
    topRight = topRight + padding;
    bottomLeft = bottomLeft - padding;

    // Add in the bounding box
    dcel->insertVertex(1, {bottomLeft.x, bottomLeft.y})->isBoundary = true;
    dcel->insertVertex(2, {topRight.x, bottomLeft.y})->isBoundary = true;
    dcel->insertVertex(3, {topRight.x, topRight.y})->isBoundary = true;
    dcel->insertVertex(4, {bottomLeft.x, topRight.y})->isBoundary = true;
//    dcel->insertEdge(bl, br);
//    dcel->insertEdge(br, tr);
//    dcel->insertEdge(tr, tl);
//    dcel->insertEdge(tl, bl);

    dcel->topRightBounds.x = topRight.x;
    dcel->topRightBounds.y = topRight.y;
    dcel->bottomLeftBounds.x = bottomLeft.x;
    dcel->bottomLeftBounds.y = bottomLeft.y;
    dcel->majorAxis = (majorAxis * (1 + 2 * BOUNDING_BOX_PADDING)) * 0.55;
    dcel->centroid = centroid;

    for (auto &p: vertexPairs) {
        if (p->v1 == p->v2) continue;

        assert(p->angle != QUIET_NAN);
        assert(p->v1 != nullptr);

        if (vertices.find(p->v1) == vertices.end()) {
            // Vert 1 is a site event origin
            if (p->v2 == nullptr) {
                // Both vertices are unbounded

                if (p->v1->y() == INFINITY) {
                    // Vertical unbounded line
                    assert(softEquals(std::abs(p->angle), M_PI / 2));
                    p->v1->pos.y = topRight.y;
                    p->v2 = dcel->insertVertex({p->v1->x(), bottomLeft.y});
                } else {
                    // Vert 1 is a site event origin, and is a full unbounded line
                    Vec2 new1 = rayIntersectBox(p->v1->pos, p->angle + M_PI, bottomLeft, topRight);
                    Vec2 new2 = rayIntersectBox(p->v1->pos, p->angle, bottomLeft, topRight);
                    p->v1 = dcel->insert(Vertex::boundary(new1));
                    p->v2 = dcel->insert(Vertex::boundary(new2));
                }
            } else {
                // Vert 1 is a site event origin, vert 2 is a voronoi vertex
                assert(p->v2 != nullptr);

                // Rectify an unbounded y at v1
                if (p->v1->pos.isInfinite) {
                    assert(p->v1->x() != INFINITY && p->v1->y() == INFINITY);

                    p->v1->pos.y = p->v2->y();
                    p->v1->pos.isInfinite = p->v2->y() == INFINITY;
                }

                if (p->v1->x() >= p->v2->x()) {
                    Vec2 boxIntersect = rayIntersectBox(p->v2->pos, p->angle, bottomLeft, topRight);
                    p->v1 = dcel->insert(Vertex::boundary(boxIntersect));
                } else {
                    Vec2 boxIntersect = rayIntersectBox(p->v2->pos, p->angle + M_PI, bottomLeft, topRight);
                    p->v1 = dcel->insert(Vertex::boundary(boxIntersect));
                }
            }
        } else if (p->v2 == nullptr) {
            // Vert 1 is a voronoi vertex, but is unbounded
            p->v2 = dcel->insert(Vertex::boundary(rayIntersectBox(p->v1->pos, p->angle, bottomLeft, topRight)));
        } else {
            // Vert 1 is a voronoi vertex, and vert 2 is non null
            // so vert 2 can't be a site event origin, and thus is another voronoi vertex
            assert(vertices.find(p->v2) != vertices.end());
        }

        dcel->insertEdge(p->v1, p->v2);
    }

    return dcel;
}

void DCELFactory::offerVertex(Vertex* vertex) {
    printf("Factory was offered vertex %s: %s\n", vertex->toString().c_str(), vertex->pos.toString());
    vertices.insert(vertex);
}

void DCELFactory::offerPair(VertexPair* vertexPair) {
    Vertex* v1 = vertexPair->v1;
    Vertex* v2 = vertexPair->v2;
    printf("Factory was offered vertex pair: <\n\tFROM %s\n\tTO\t %s\n> with angle %f rad (%f deg)\n",
           v1 == nullptr ? "null" : (v1->toString() + " " + v1->pos.toString()).c_str(),
           v2 == nullptr ? "null" : (v2->toString() + " " + v2->pos.toString()).c_str(),
           vertexPair->angle,
           vertexPair->angle / M_PI * 180.0
    );
    vertexPairs.insert(vertexPair);
}

int DCELFactory::numVertices() {
    return static_cast<int>(vertices.size());
}

