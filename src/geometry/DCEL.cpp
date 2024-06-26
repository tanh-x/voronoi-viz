#include <algorithm>
#include <set>
#include <cassert>
#include <cmath>
#include <unordered_map>
#include "geometry/DCEL.hpp"
#include "utils/math/mathematics.hpp"

// Any reasonable number (around 0.1 to 0.5), aesthetics only
#define BOUNDING_BOX_PADDING 0.362160297

Vertex* DCEL::insertVertex(int label, Vec2 position) {
    auto* v = new Vertex(label, position);
//    v->pos.x = std::clamp(v->pos.x, bottomLeftBounds.x, topRightBounds.x);
//    v->pos.y = std::clamp(v->pos.y, bottomLeftBounds.y, topRightBounds.y);
    return insert(v);
}

HalfEdge* DCEL::insertEdge(Vertex* v1, Vertex* v2) {
    auto* e = new HalfEdge(v1, v2);
    if (v1->isBoundary || v2->isBoundary) e->unbounded = true;
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

HalfEdge* DCEL::insert(HalfEdge* edge) {
    halfEdges.push_back(edge);
    return edge;
}


Face* DCEL::insert(Face* face) {
    faces.push_back(face);
    return face;
}


void DCEL::printOutputVoronoiStyle() {
    // Print vertices
    printf("\n");
    for (Vertex* v: vertices) {
        printf(
            "%s %s %s\n",
            v->toString().c_str(),
            v->pos.toString(),
            (v->incidentEdge == nullptr ? "nil" : v->incidentEdge->toString())
        );
    }

    // Print faces
    printf("\n");
    for (Face* f: faces) {
        printf(
            "c%d %s%s %s%s\n",
            f->label,
            f->outer == nullptr ? "" : "e",
            f->outer == nullptr ? "nil" : f->outer->toString(),
            f->inner == nullptr ? "" : "e",
            f->inner == nullptr ? "nil" : f->inner->toString()
        );
    }

    // Print vertices
    printf("\n");
    for (HalfEdge* e: halfEdges) {
        printf(
            "e%s %s e%s %s e%s e%s\n",
            e->toString(),
            e->origin->toString().c_str(),
            e->twin->toString(),
            (e->incidentFace == nullptr ? "nil" : e->incidentFace->toString().c_str()),
            (e->next == nullptr ? "nil" : e->next->toString()),
            (e->prev == nullptr ? "nil" : e->prev->toString())
        );
    }
}

void DCEL::printOutputDelaunayStyle() {
    // Print vertices
    printf("\n");
    for (Vertex* v: vertices) {
        printf(
            "p%d %s %s\n",
            v->label,
            v->pos.toString(),
            (v->incidentEdge == nullptr ? "nil" : v->incidentEdge->toString())
        );
    }

    // Print faces
    printf("\n");
    for (Face* f: faces) {
        if (f->unbounded) {
            printf(
                "uf %s %s\n",
                f->outer == nullptr ? "nil" : f->outer->toString(),
                f->inner == nullptr ? "nil" : f->inner->toString()
            );
        } else {
            printf(
                "t%d %s %s\n",
                f->label,
                f->outer == nullptr ? "nil" : f->outer->toString(),
                f->inner == nullptr ? "nil" : f->inner->toString()
            );
        }
    }

    // Print vertices
    printf("\n");
    for (HalfEdge* e: halfEdges) {
        printf(
            "d%s p%d d%s %s d%s d%s\n",
            e->toString(),
            e->origin->label,
            e->twin->toString(),
            (e->incidentFace == nullptr
             ? "nil"
             : (e->incidentFace->unbounded ? "uf" : "t" + std::to_string(e->incidentFace->label)).c_str()),
            (e->next == nullptr ? "nil" : e->next->toString()),
            (e->prev == nullptr ? "nil" : e->prev->toString())
        );
    }
}


struct VertexPairComparator {
    bool operator()(const VertexPair* lhs, const VertexPair* rhs) const {
        // Order-independent comparison of vertex pointers
        std::pair<Vertex*, Vertex*> minmax_lhs = std::minmax(lhs->v1, lhs->v2);
        std::pair<Vertex*, Vertex*> minmax_rhs = std::minmax(rhs->v1, rhs->v2);
        return minmax_lhs < minmax_rhs;
    }
};


DCELFactory::DCELFactory(const std::vector<Vec2> &sites) : sites(sites) {
    dcel = new DCEL();
    for (auto s: sites) {
        Face* newFace = dcel->insert(new Face(&s));
        assert(newFace->label == s.identifier);
        cells.insert({s.identifier, newFace});
    }
}


DCEL* DCELFactory::createDCEL(const std::vector<Vec2> &sites) {
    // Define the bounding box corner coordinates
    bottomLeft = Vec2(DOUBLE_INFINITY, DOUBLE_INFINITY);
    topRight = Vec2(-DOUBLE_INFINITY, -DOUBLE_INFINITY);

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
    bl = dcel->insert(Vertex::boundary({bottomLeft.x, bottomLeft.y}, 1));
    br = dcel->insert(Vertex::boundary({topRight.x, bottomLeft.y}, 2));
    tr = dcel->insert(Vertex::boundary({topRight.x, topRight.y}, 3));
    tl = dcel->insert(Vertex::boundary({bottomLeft.x, topRight.y}, 4));
    numBoundaryVertices = 4;

    dcel->topRightBounds.x = topRight.x;
    dcel->topRightBounds.y = topRight.y;
    dcel->bottomLeftBounds.x = bottomLeft.x;
    dcel->bottomLeftBounds.y = bottomLeft.y;
    dcel->majorAxis = (majorAxis * (1 + 2 * BOUNDING_BOX_PADDING)) * 0.5;
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
                    Vertex* new1 = getOrCreateBoundaryVertex(p->v1->pos, p->angle + M_PI);
                    Vertex* new2 = getOrCreateBoundaryVertex(p->v1->pos, p->angle);
                    p->v1 = dcel->insert(new1);
                    p->v2 = dcel->insert(new2);
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

                double rayAngle = p->angle;
                if (p->v1->x() < p->v2->x()) rayAngle += M_PI;
                p->v1 = dcel->insert(getOrCreateBoundaryVertex(p->v2->pos, rayAngle));
            }
        } else if (p->v2 == nullptr) {
            // Vert 1 is a voronoi vertex, but is unbounded
            p->v2 = dcel->insert(getOrCreateBoundaryVertex(p->v1->pos, p->angle));
        } else {
            // Vert 1 is a voronoi vertex, and vert 2 is non null
            // so vert 2 can't be a site event origin, and thus is another voronoi vertex
            assert(vertices.find(p->v2) != vertices.end());
        }

        HalfEdge* newHalfEdge = dcel->insertEdge(p->v1, p->v2);
        HalfEdge* twinEdge = newHalfEdge->generateTwin();
        fwdEdges.push_back(newHalfEdge);
        dcel->insert(twinEdge);
        newHalfEdge->bindTwins(twinEdge);

        // Decide which incident face to use
        Vec2 dir = newHalfEdge->dest->pos - newHalfEdge->origin->pos;
        Vec2 dirA = (*p->incidentSiteA) - newHalfEdge->origin->pos;
        Vec2 dirB = (*p->incidentSiteB) - newHalfEdge->origin->pos;

        if (dir.cross(dirA) > 0) {
            assert(dir.cross(dirB) - NUMERICAL_TOLERANCE <= 0);
            newHalfEdge->incidentFace = cells.at(p->incidentSiteA->identifier);
            twinEdge->incidentFace = cells.at(p->incidentSiteB->identifier);
        } else {
            assert(dir.cross(dirA) - NUMERICAL_TOLERANCE <= 0);
            newHalfEdge->incidentFace = cells.at(p->incidentSiteB->identifier);
            twinEdge->incidentFace = cells.at(p->incidentSiteA->identifier);
        }

        // Face relation operations
        newHalfEdge->incidentFace->offerComponent(newHalfEdge);
        twinEdge->incidentFace->offerComponent(twinEdge);
    }

    printf(
        "Pushed all preliminary vertices and edges into DCEL, with %d vertices and %d edges\n",
        dcel->numVertices(), dcel->numHalfEdges()
    );

    return consolidateDCEL(dcel);
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

Vertex* DCELFactory::getOrCreateBoundaryVertex(Vec2 origin, double angle) {
    Vec2 intersect = rayIntersectBox(origin, angle, bottomLeft, topRight);

    if (softEquals(intersect, bl->pos)) return bl;
    if (softEquals(intersect, br->pos)) return br;
    if (softEquals(intersect, tr->pos)) return tr;
    if (softEquals(intersect, tl->pos)) return tl;

    return Vertex::boundary(intersect, ++numBoundaryVertices);
}


void printIncidenceSet(std::set<HalfEdge*, HalfEdgeAngleComparator>* incidenceSet) {
    for (HalfEdge* edge: *incidenceSet) {
        printf("%s - %f (%f deg)\n", edge->toString(), edge->angle, edge->angle / M_PI * 180);
    }
}

DCEL* DCELFactory::consolidateDCEL(DCEL* geometry) {
    std::unordered_map<Vertex*, std::set<HalfEdge*, HalfEdgeAngleComparator>*> incidenceMap {};

    // Set up the incidence map
    for (Vertex* v: geometry->vertices) {
        auto* newSet = new std::set<HalfEdge*, HalfEdgeAngleComparator>();
        incidenceMap.insert({v, newSet});
    }

    // Go through each outer and update the incidence map
    for (HalfEdge* fwdEdge: geometry->halfEdges) {
        try {
            incidenceMap.at(fwdEdge->origin)->insert(fwdEdge);
        } catch (std::out_of_range const &) {
            continue;
        }
    }

    // Now, work out the next/prev relationship for each vertex
    for (Vertex* v: geometry->vertices) {
        std::set<HalfEdge*, HalfEdgeAngleComparator>* incidenceSet = incidenceMap.at(v);
//        printIncidenceSet(incidenceSet);

        // Check if there are more than 2 edges through this vertex
        if (incidenceSet->empty()) continue;

        if (incidenceSet->size() == 1) {
            HalfEdge* edge = *incidenceSet->begin();
            v->incidentEdge = edge;
            edge->twin->chainNext(edge);
            continue;
        }

        // Establish the prev/next outer relation
        HalfEdge* edge = *incidenceSet->rbegin();
        assert(edge != nullptr);
        v->incidentEdge = edge;
        for (HalfEdge* nextEdge: *incidenceSet) {
            nextEdge->twin->chainNext(edge);
            edge = nextEdge;
        }
    }

    geometry->consolidated = true;

    return geometry;
}


DCEL* DCELFactory::buildDualGraph() {
    assert(dcel != nullptr);
    assert(dcel->consolidated);
    assert(dcel->numVertices() > 0);
    assert(dcel->numEdges() > 0);
    assert(dcel->numFaces() > 0);

    DCEL* dualGraph = new DCEL();
    bottomLeft = Vec2(DOUBLE_INFINITY, DOUBLE_INFINITY);
    topRight = Vec2(-DOUBLE_INFINITY, -DOUBLE_INFINITY);

    std::unordered_map<int, Vertex*> siteVertices;
    for (Vec2 s: sites) {
        auto* newVertex = new Vertex(s.identifier, s);
        dualGraph->insert(newVertex);

        siteVertices.insert({s.identifier, newVertex});

        // Also calculate the bounding box
        bottomLeft.x = std::min(bottomLeft.x, s.x);
        bottomLeft.y = std::min(bottomLeft.y, s.y);
        topRight.x = std::max(topRight.x, s.x);
        topRight.y = std::max(topRight.y, s.y);
    }

    // Calculate bounding box
    double width = topRight.x - bottomLeft.x;
    double height = topRight.y - bottomLeft.y;
    double majorAxis = std::max(width, height);

    Vec2 centroid = {(topRight.x + bottomLeft.x) * 0.5, (topRight.y + bottomLeft.y) * 0.5};
    topRight = centroid + Vec2(majorAxis, majorAxis) * 0.5;
    bottomLeft = centroid - Vec2(majorAxis, majorAxis) * 0.5;

    dualGraph->topRightBounds.x = topRight.x;
    dualGraph->topRightBounds.y = topRight.y;
    dualGraph->bottomLeftBounds.x = bottomLeft.x;
    dualGraph->bottomLeftBounds.y = bottomLeft.y;
    dualGraph->majorAxis = (majorAxis * (1 + 2 * BOUNDING_BOX_PADDING)) * 0.5;
    dualGraph->centroid = centroid;

    // Map between Voronoi vertices and triangulation faces
    std::unordered_map<Vertex*, Face*> triangleMap;
    for (auto &v: dcel->vertices) {
        if (v->isBoundary) continue;
        auto* newFace = new Face(new Vec2(v->pos.x, v->pos.y, dualGraph->numFaces() + 1));
        dualGraph->insert(newFace);
        triangleMap.insert({v, newFace});
    }
    auto* unboundedFace = new Face();
    unboundedFace->unbounded = true;
    dualGraph->insert(unboundedFace);

    for (auto &edge: fwdEdges) {
        try {
            Face* leftOuterFace = edge->dest->isBoundary ? unboundedFace : triangleMap.at(edge->dest);
            Face* rightOuterFace = edge->origin->isBoundary ? unboundedFace : triangleMap.at(edge->origin);

            Face* leftFace = edge->incidentFace;
            Face* rightFace = edge->twin->incidentFace;
            if (leftFace == rightFace) continue;

            HalfEdge* dualEdge = new HalfEdge(siteVertices.at(leftFace->label), siteVertices.at(rightFace->label));
            HalfEdge* twinEdge = dualEdge->generateTwin();

            dualEdge->incidentFace = leftOuterFace;
            twinEdge->incidentFace = rightOuterFace;
            leftOuterFace->offerComponent(dualEdge);
            rightOuterFace->offerComponent(twinEdge);

            dualEdge->bindTwins(twinEdge);
            dualGraph->insert(dualEdge);
            dualGraph->insert(twinEdge);
        } catch (std::out_of_range const &) {
            continue;
        }
    }

    consolidateDCEL(dualGraph);

    return dualGraph;
}
