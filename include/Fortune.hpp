#ifndef FORTUNE_HPP
#define FORTUNE_HPP

#include <vector>
#include <cassert>
#include "geometry/DCEL.hpp"
#include "utils/PriorityQueue.hpp"
#include "utils/SplayTree.hpp"
#include "utils/math/mathematics.hpp"

class Event;
class BeachValue;

class BeachKey {
public:
    // Arcs are leaf nodes, breakpoints lie between two arcs, and are internal nodes
    bool isArc;

    Node<BeachKey*, BeachValue*>* next {nullptr};
    Node<BeachKey*, BeachValue*>* prev {nullptr};

    double* sweepY {nullptr};

    BeachKey(double* sweepY, Vec2* focus) : isArc(true), focus(focus) {};

    BeachKey(double* sweepY, Vec2* left, Vec2* right) : isArc(false), leftArc(left), rightArc(right) {};

    [[nodiscard]] double fieldOrdering(double t) const;

    bool operator<(const BeachKey &other) const;

    // If arc node, then store the defining site/focus
    Vec2* focus {nullptr};

    // If breakpoint node, then store the two defining sites/foci/arcs
    Vec2* leftArc {nullptr};
    Vec2* rightArc {nullptr};
};

struct BeachValue {
    VertexPair* breakpointEdge = nullptr;
    Event* circleEvent = nullptr;

    static BeachValue* breakpointPtr(VertexPair* ptr);

    static BeachValue* arcPtr(Event* ptr);
};

class Event {
public:
    Vec2 pos;
    bool isSiteEvent;
    Node<BeachKey*, BeachValue*>* arcNode;

    bool isInvalidated = false;

    explicit Event(Vec2 site)
        : pos(site),
          isSiteEvent(true),
          arcNode {nullptr} {}

    Event(Vec2 circleBottom, Node<BeachKey*, BeachValue*>* arcNode) :
        pos(circleBottom),
        isSiteEvent(false),
        arcNode(arcNode) {}

    bool operator<(const Event &other) const {
        // Sort events by y-coordinate
        return pos.y < other.pos.y
               // ...and prioritize site events
               || (pos.y == other.pos.y && isSiteEvent && !other.isSiteEvent);
    }
};


// Main function to compute the Voronoi diagram
DCEL* computeVoronoi(const std::vector<Vec2> &sites);

// Processing a site event
void processSiteEvent(
    Event* event,
    SplayTree<BeachKey*, BeachValue*> &beachLine,
    PriorityQueue<Event*> &eventQueue,
    DCELFactory* dcel,
    double* sweepY
);

// Processing a circle event
void processCircleEvent(
    Event* event,
    SplayTree<BeachKey*, BeachValue*> &beachLine,
    PriorityQueue<Event*> &eventQueue,
    DCELFactory* dcel,
    double* sweepY
);

void finalizeEdges(
    SplayTree<BeachKey*, BeachValue*> &beachLine,
    DCELFactory* dcel
);

void checkCircleEvent(
    Node<BeachKey*, BeachValue*>* arcNode,
    const double* sweepY,
    PriorityQueue<Event*> &eventQueue
);


#endif
