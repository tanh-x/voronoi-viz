
#ifndef VORONOI_VIZ_DCEL_HPP
#define VORONOI_VIZ_DCEL_HPP

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include "Vertex.hpp"
#include "Face.hpp"

class DCELFactory;

class DCEL {
public:
    friend class DCELFactory;

    std::vector<Vertex*> vertices;
    std::vector<HalfEdge*> halfEdges;
    std::vector<Face*> faces;
    std::vector<Vec2> sites;

    Vec2 bottomLeftBounds {Vec2(-DOUBLE_INFINITY, -DOUBLE_INFINITY)};
    Vec2 topRightBounds {Vec2(DOUBLE_INFINITY, DOUBLE_INFINITY)};
    double majorAxis = DOUBLE_INFINITY;
    Vec2 centroid {Vec2(0, 0)};

    [[nodiscard]] int numVertices() const;

    [[nodiscard]] int numHalfEdges() const;

    [[nodiscard]] int numEdges() const;

    [[nodiscard]] int numFaces() const;

    ~DCEL() {
        for (auto v: vertices) delete v;
        for (auto e: halfEdges) delete e;
        for (auto f: faces) delete f;
    }


    [[nodiscard]] double getCenteredX(double x) const;

    [[nodiscard]] double getCenteredY(double y) const;

    void printOutput();

private:
    Vertex* insertVertex(int label, Vec2 position);

    Vertex* insertVertex(Vec2 position);

    Vertex* insert(Vertex* vertex);

    HalfEdge* insert(HalfEdge* edge);

    Face* insert(Face* face);

    HalfEdge* insertEdge(Vertex* v1, Vertex* v2);

    HalfEdge* insertEdge(Vertex* v1, Vertex* v2, double angle);

};

class DCELFactory {
public:
    explicit DCELFactory(const std::vector<Vec2> &sites);;

    void offerVertex(Vertex* vertex);

    void offerPair(VertexPair* vertexPair);

    int numVertices();

    DCEL* createDCEL(const std::vector<Vec2> &sites);

    DCEL* consolidateDCEL();

private:
    DCEL* dcel {};

    const std::vector<Vec2> &sites;

    Vec2 bottomLeft = Vec2(DOUBLE_INFINITY, DOUBLE_INFINITY);
    Vec2 topRight = Vec2(-DOUBLE_INFINITY, -DOUBLE_INFINITY);

    int numBoundaryVertices = 0;

    Vertex* bl = nullptr;
    Vertex* br = nullptr;
    Vertex* tr = nullptr;
    Vertex* tl = nullptr;

    std::unordered_set<Vertex*> vertices {};
    std::unordered_set<VertexPair*> vertexPairs {};
    std::unordered_map<Vertex*, std::set<HalfEdge*, HalfEdgeAngleComparator>*> incidenceMap {};
    std::unordered_map<int, Face*> cells {};

    Vertex* getOrCreateBoundaryVertex(Vec2 origin, double angle);

    void consolidateEdges();

    void consolidateVertices();

    void consolidateFaces();
};

#endif //VORONOI_VIZ_DCEL_HPP
