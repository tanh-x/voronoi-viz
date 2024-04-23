#ifndef VORONOI_VIZ_HALFEDGE_HPP
#define VORONOI_VIZ_HALFEDGE_HPP

#include "Vertex.hpp"
#include "Face.hpp"

class Face;

class Vertex;

class HalfEdge {
public:
    Vertex* origin;
    HalfEdge* twin;
    HalfEdge* next;
    HalfEdge* prev;
    Face* incidentFace;

    HalfEdge() : origin(nullptr), twin(nullptr), next(nullptr), prev(nullptr), incidentFace(nullptr) {}
};

#endif //VORONOI_VIZ_HALFEDGE_HPP
