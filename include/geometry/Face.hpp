#ifndef VORONOI_VIZ_FACE_HPP
#define VORONOI_VIZ_FACE_HPP

#include "HalfEdge.hpp"

class HalfEdge;

class Face {
public:
    // Outer component
    HalfEdge* edge;

    Face() : edge(nullptr) {}
};


#endif //VORONOI_VIZ_FACE_HPP
