#include <limits>
#include <cmath>
#include "Fortune.hpp"

DCEL* computeVoronoi(const std::vector<Vec2> &sites) {
    auto dcel = new DCELFactory();
    PriorityQueue<Event*> eventQueue;
    SplayTree<BeachKey*, BeachValue*> beachLine;

    for (const Vec2 &site: sites) eventQueue.add(new Event(site));

    double sweepY = eventQueue.peek()->pos.y;

    // Main loop to process all events
    while (!eventQueue.empty()) {
        Event* event = eventQueue.poll();
        sweepY = event->pos.y;
        if (event->isSiteEvent) {
            processSiteEvent(event, beachLine, eventQueue, dcel, &sweepY);
        } else {
            processCircleEvent(event, beachLine, eventQueue, dcel, &sweepY);
        }
//        delete event;
    }

    finalizeEdges(beachLine, dcel);
    return dcel->createDCEL();
}

void processSiteEvent(
    Event* event,
    SplayTree<BeachKey*, BeachValue*> &beachLine,
    PriorityQueue<Event*> &eventQueue,
    DCELFactory* dcel,
    double* sweepY
) {
    assert(event->isSiteEvent);
    // Extract the site point from the event
    auto* newArc = new BeachKey(sweepY, &event->pos);

    // Find the arc directly above the new site point
    Node<BeachKey*, BeachValue*>* arcAboveNode = beachLine.root;
    while (arcAboveNode) {
        if (arcAboveNode->key->isArc) break;
        // Use the defined natural field ordering
        arcAboveNode = (arcAboveNode->key > newArc) ? arcAboveNode->right : arcAboveNode->left;
    }

    // If no arc is found directly above, it means this is the first site
    if (!arcAboveNode) {
        assert(beachLine.root == nullptr);
        beachLine.add(newArc, BeachValue::arcPtr(nullptr));
        return;  // Early return; no further action needed if this is the first site
    }

    BeachKey* arcAbove = arcAboveNode->key;

    assert(arcAbove->isArc);

    // Create new arcs from the split of the old arc
    auto* leftArc = new BeachKey(sweepY, arcAbove->focus);
    auto* rightArc = new BeachKey(sweepY, arcAbove->focus);
    // TODO: For degeneracy, use the left and right arc instead of the same arc.

    // Create two new breakpoints
    auto* leftBreakpoint = new BeachKey(sweepY, leftArc->focus, newArc->focus);
    auto* rightBreakpoint = new BeachKey(sweepY, newArc->focus, rightArc->focus);

    // Create five new nodes corresponding to two breakpoints and three arcs
    auto* newArcNode = new Node<BeachKey*, BeachValue*>(newArc, BeachValue::arcPtr(nullptr));
    auto* leftArcNode = new Node<BeachKey*, BeachValue*>(leftArc, BeachValue::arcPtr(nullptr));
    auto* rightArcNode = new Node<BeachKey*, BeachValue*>(rightArc, BeachValue::arcPtr(nullptr));
    auto* leftBpNode = new Node<BeachKey*, BeachValue*>(leftBreakpoint, BeachValue::breakpointPtr(new VertexPair()));
    auto* rightBpNode = new Node<BeachKey*, BeachValue*>(rightBreakpoint, BeachValue::breakpointPtr(new VertexPair()));


    // Set up the subtree structure
    leftBpNode->setRightChild(rightBpNode);
    leftBpNode->setLeftChild(leftArcNode);
    rightBpNode->setLeftChild(newArcNode);
    rightBpNode->setRightChild(rightArcNode);

    // Place it back into the tree
    auto subtreeParent = arcAboveNode->parent;

    if (subtreeParent == nullptr) { beachLine.root = leftBpNode; }
    else if (subtreeParent->left == arcAboveNode) {
        subtreeParent->setLeftChild(leftBpNode);
//        beachLine.get(subtreeParent->key);  // Trigger a splay
    } else {
        subtreeParent->setRightChild(leftBpNode);
//        beachLine.get(subtreeParent->key);  // Trigger a splay
    }

    // Set up the linked list pointers
    leftArc->prev = arcAbove->prev;
    leftArc->next = newArcNode;

    newArc->prev = leftArcNode;
    newArc->next = rightArcNode;

    rightArc->prev = newArcNode;
    rightArc->next = arcAbove->next;

    // Invalidate the split arc's circle event
    if (arcAboveNode->value->circleEvent != nullptr) arcAboveNode->value->circleEvent->isInvalidated = true;

    delete arcAbove;

    // Check for potential circle eventQueue caused by these new arcs
    checkCircleEvent(leftArcNode, sweepY, eventQueue);
    checkCircleEvent(rightArcNode, sweepY, eventQueue);
}


void checkCircleEvent(
    Node<BeachKey*, BeachValue*>* arcNode,
    const double* sweepY,
    PriorityQueue<Event*> &eventQueue
) {
    BeachKey* arc = arcNode->key;
    assert(arc->isArc);
    if (arc->prev == nullptr || arc->next == nullptr) return;

    // Extract positions of points
    Vec2 a = *arc->prev->key->focus;
    Vec2 b = *arc->focus;
    Vec2 c = *arc->next->key->focus;

    // Check if b is a vertex of a converging circle with a and c
//    assert(computeDeterminantTest(a, b, c) >= 0); // Points must be oriented clockwise

    // Calculate the center of the circle through a, b, and c

    Vec2 center = computeCircleCenter(a, b, c);
    double circleEventY = center.y - center.distanceTo(a);

    // Only consider this event if it is below the sweep line
    if (circleEventY < *sweepY) {
        // Two way reference between the node and the event
        auto* circleEvent = new Event({center.x, circleEventY}, arcNode);
        arcNode->value->circleEvent = circleEvent;

        // Add it to the event queue
        eventQueue.add(circleEvent);
    }
}


void processCircleEvent(
    Event* event,
    SplayTree<BeachKey*, BeachValue*> &beachLine,
    PriorityQueue<Event*> &eventQueue,
    DCELFactory* dcel,
    double* sweepY
) {
    if (event->isInvalidated) return;

    // Get arc node corresponding to the circle event
    assert(!event->isSiteEvent);
    Node<BeachKey*, BeachValue*>* arcNode = event->arcNode;
    assert(arcNode != nullptr);
    BeachKey* arc = arcNode->key;
    assert(arc->isArc);

    // Add the center of the circle as a new Voronoi vertex
    auto* newVertex = new Vertex(999, event->pos);
    dcel->vertices.insert(newVertex);
//    arc->leftBp->value->breakpointEdge->offerVertex(newVertex);
//    arc->rightBp->value->breakpointEdge->offerVertex(newVertex);
//    arcNode->value->breakpointEdge->offerVertex(newVertex);

    // Remove the disappearing arc from the beach line
    beachLine.replace(arcNode, nullptr);

    // Connect the disappearing arc's neighbors in the beach line
    assert(arc->prev != nullptr);
    assert(arc->next != nullptr);

    // Delete affected events
    if (arc->prev->value->circleEvent != nullptr) arc->prev->value->circleEvent->isInvalidated = true;
    if (arc->next->value->circleEvent != nullptr) arc->next->value->circleEvent->isInvalidated = true;

    BeachKey* prevArc = arc->prev->key;
    BeachKey* nextArc = arc->next->key;
    prevArc->next = arc->next;
    nextArc->prev = arc->prev;



    // Check adjacent arcs for additional circle events
    checkCircleEvent(arc->prev, sweepY, eventQueue);
    checkCircleEvent(arc->next, sweepY, eventQueue);

    // Clean up the removed arc
//    delete arc;
//    delete arcNode;
}

void finalizeEdges(
    SplayTree<BeachKey*, BeachValue*> &beachLine,
    DCELFactory* dcel
) {
    // Close off any unbounded half vertexPairs
}

double BeachKey::fieldOrdering(double t) const {
    if (isArc) {
        assert(focus);
        assert(leftArc == nullptr);
        assert(rightArc == nullptr);
        return focus->x;
    } else {
        assert(focus == nullptr);
        assert(leftArc);
        assert(rightArc);
        return pointDirectrixIntersectionX(*leftArc, *rightArc, t);
    }
}

bool BeachKey::operator<(const BeachKey &other) const {
    double orderingParameter = *other.sweepY;
    return this->fieldOrdering(orderingParameter) < other.fieldOrdering(orderingParameter);
}


BeachValue* BeachValue::breakpointPtr(VertexPair* ptr) {
    return new BeachValue({ptr, nullptr});
}

BeachValue* BeachValue::arcPtr(Event* ptr) {
    return new BeachValue({nullptr, ptr});
}
