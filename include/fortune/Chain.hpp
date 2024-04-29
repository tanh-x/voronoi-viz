#ifndef VORONOI_VIZ_CHAIN_HPP
#define VORONOI_VIZ_CHAIN_HPP

#include "geometry/HalfEdge.hpp"
#include "utils/math/Vec2.hpp"
#include "Event.hpp"

class Event;

class Chain {
public:
    // Arcs are leaf nodes, breakpoints lie between two arcs, and are internal nodes
    bool isArc;

    double* sweepY {nullptr};

    Chain(double* sweepY, Vec2* focus)
        : isArc(true),
          sweepY(sweepY),
          focus(focus) {};

    Chain(double* sweepY, Vec2* left, Vec2* right)
        : isArc(false),
          sweepY(sweepY),
          leftSite(left),
          rightSite(right) {};

    [[nodiscard]] double fieldOrdering(double t) const;

    // If arc node, then store the defining site/focus
    Vec2* focus {nullptr};

    // If breakpoint node, then store the two defining sites/foci/arcs
    Vec2* leftSite {nullptr};
    Vec2* rightSite {nullptr};

    [[nodiscard]] const char* toString() const;

};

class TreeValueFacade {
public:
    VertexPair* breakpointEdge = nullptr;
    Event* circleEvent = nullptr;

    static TreeValueFacade* breakpointPtr(VertexPair* ptr);

    static TreeValueFacade* arcPtr(Event* ptr);
};

struct ChainComparator {
    bool operator()(Chain* a, Chain* b) const;
};


#endif //VORONOI_VIZ_CHAIN_HPP
