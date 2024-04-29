
#ifndef VORONOI_VIZ_EVENT_HPP
#define VORONOI_VIZ_EVENT_HPP

#include "Chain.hpp"
#include "utils/LinkedSplayTree.hpp"

class Chain;

class TreeValueFacade;

class Event {
public:
    Vec2 pos;
    bool isSiteEvent;
    LinkedNode<Chain*, TreeValueFacade*>* arcNode;

    Vec2 circleCenter {Vec2::infinity()};

    bool isInvalidated = false;

    explicit Event(Vec2 site)
        : pos(site),
          isSiteEvent(true),
          arcNode {nullptr} {}

    Event(Vec2 circleBottom, Vec2 center, LinkedNode<Chain*, TreeValueFacade*>* arcNode) :
        pos(circleBottom),
        isSiteEvent(false),
        arcNode(arcNode),
        circleCenter(center) {}


    [[nodiscard]] double x() const;

    [[nodiscard]] double y() const;
};


struct EventComparator {
    bool operator()(Event* a, Event* b) const;
};


#endif //VORONOI_VIZ_EVENT_HPP

