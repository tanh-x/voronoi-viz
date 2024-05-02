#ifndef VORONOI_VIZ_FACE_HPP
#define VORONOI_VIZ_FACE_HPP

#include "HalfEdge.hpp"

class HalfEdge;

class Face {
public:
    int label;

    // Voronoi cell (optional)
    Vec2* site {nullptr};

    HalfEdge* outer {nullptr};
    HalfEdge* inner {nullptr};
    bool unbounded = false;

    Face() : label(-1) {}

//    Face(HalfEdge* outerComponent, int label) : label(label), outer(outerComponent) {}

    explicit Face(Vec2* site) : label(site->identifier), site(site) {}

    [[nodiscard]] std::string toString() const;

    void offerComponent(HalfEdge* edge);
};


#endif //VORONOI_VIZ_FACE_HPP
