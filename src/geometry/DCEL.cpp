#include "geometry/DCEL.hpp"

Vertex* DCEL::insertVertex(int id, Vec2 position) {
    auto* v = new Vertex(id, position);
    vertices.push_back(v);
    return v;
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

DCEL* DCELFactory::createDCEL() {
    auto* dcel = new DCEL();
    for (auto &v: vertices) dcel->vertices.push_back(v);
    for (auto &p: vertexPairs) dcel->insertEdge(p->v1, p->v2);

    return dcel;
}
