
#ifndef VORONOI_VIZ_DCEL_HPP
#define VORONOI_VIZ_DCEL_HPP

#include <vector>
#include <unordered_set>
#include "Vertex.hpp"
#include "Face.hpp"

class DCELFactory;

class DCEL {
public:
    friend class DCELFactory;


    std::vector<Vertex*> vertices;
    std::vector<HalfEdge*> halfEdges;
    std::vector<Face*> faces;

    Vec2 boundingBottomLeft {Vec2(-DOUBLE_INFINITY, -DOUBLE_INFINITY)};
    Vec2 boundingTopRight {Vec2(DOUBLE_INFINITY, DOUBLE_INFINITY)};

    [[nodiscard]] int numVertices() const;

    [[nodiscard]] int numHalfEdges() const;

    [[nodiscard]] int numEdges() const;

    [[nodiscard]] int numFaces() const;

    ~DCEL() {
        for (auto v: vertices) delete v;
        for (auto e: halfEdges) delete e;
        for (auto f: faces) delete f;
    }

private:
    Vertex* insertVertex(int id, Vec2 position);

    HalfEdge* insertEdge(Vertex* v1, Vertex* v2);
};

class DCELFactory {
public:
    std::unordered_set<Vertex*> vertices {};
    std::unordered_set<VertexPair*> vertexPairs {};

    DCEL* createDCEL(const std::vector<Vec2> &sites);

    DCELFactory() = default;
};

#endif //VORONOI_VIZ_DCEL_HPP
