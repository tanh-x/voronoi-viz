#ifndef VORONOI_VIZ_FACE_HPP
#define VORONOI_VIZ_FACE_HPP

#include "HalfEdge.hpp"

class HalfEdge;

class Face {
public:
    int label;

    // Voronoi cell (optional)
    Vec2* site {nullptr};

    // Outer component
    HalfEdge* edge {nullptr};

    Face() : label(-1) {}

    Face(HalfEdge* outerComponent, int label) : label(label), edge(outerComponent) {}

    explicit Face(Vec2* site) : label(site->identifier), site(site) {}

    [[nodiscard]] std::string toString() const;
};


#endif //VORONOI_VIZ_FACE_HPP
