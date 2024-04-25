#ifndef FORTUNE_HPP
#define FORTUNE_HPP

#include <vector>
#include <cassert>
#include "geometry/DCEL.hpp"
#include "utils/PriorityQueue.hpp"
#include "utils/SplayTree.hpp"
#include "utils/math/mathematics.hpp"
#include "utils/LinkedSplayTree.hpp"

#define NUMERICAL_TOLERANCE 1e-7

class Event;

class BeachValue;

class BeachKey {
public:
    // Arcs are leaf nodes, breakpoints lie between two arcs, and are internal nodes
    bool isArc;

    double* sweepY {nullptr};

    BeachKey(double* sweepY, Vec2* focus)
        : isArc(true),
          sweepY(sweepY),
          focus(focus) {};

    BeachKey(double* sweepY, Vec2* left, Vec2* right)
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

    [[nodiscard]] std::string toString() const {
        if (isArc) return "ARC[" + std::to_string(focus->identifier) + "]";
        else return "BP[" + std::to_string(leftSite->identifier) + "," + std::to_string(rightSite->identifier) + "]";
    }
};

class BeachValue {
public:
    VertexPair* breakpointEdge = nullptr;
    Event* circleEvent = nullptr;

    static BeachValue* breakpointPtr(VertexPair* ptr);

    static BeachValue* arcPtr(Event* ptr);
};

class Event {
public:
    Vec2 pos;
    bool isSiteEvent;
    LinkedNode<BeachKey*, BeachValue*>* arcNode;

    Vec2 circleCenter {Vec2::infinity()};

    bool isInvalidated = false;

    explicit Event(Vec2 site)
        : pos(site),
          isSiteEvent(true),
          arcNode {nullptr} {}

    Event(Vec2 circleBottom, Vec2 center, LinkedNode<BeachKey*, BeachValue*>* arcNode) :
        pos(circleBottom),
        isSiteEvent(false),
        arcNode(arcNode),
        circleCenter(center) {}
};


struct EventComparator {
    bool operator()(Event* a, Event* b) const { return a->pos.y > b->pos.y; }
};

struct BeachLineComparator {
    bool operator()(BeachKey* a, BeachKey* b) const {
        assert(a->sweepY == b->sweepY);
        double orderingParameter = *a->sweepY;
        return a->fieldOrdering(orderingParameter) < b->fieldOrdering(orderingParameter);
    }
};


// Main function to compute the Voronoi diagram
DCEL* computeVoronoi(const std::vector<Vec2> &sites);

// Processing a site event
void processSiteEvent(
    Event* event,
    LinkedSplayTree<BeachKey*, BeachValue*, BeachLineComparator> &beachLine,
    PriorityQueue<Event*, EventComparator> &eventQueue,
    DCELFactory* dcel,
    double* sweepY
);

// Processing a circle event
void processCircleEvent(
    Event* event,
    LinkedSplayTree<BeachKey*, BeachValue*, BeachLineComparator> &beachLine,
    PriorityQueue<Event*, EventComparator> &eventQueue,
    DCELFactory* dcel,
    double* sweepY
);

void finalizeEdges(
    LinkedSplayTree<BeachKey*, BeachValue*, BeachLineComparator> &beachLine,
    DCELFactory* dcel
);

void checkCircleEvent(
    LinkedNode<BeachKey*, BeachValue*>* arcNode,
    const double* sweepY,
    PriorityQueue<Event*, EventComparator> &eventQueue
);


#endif
