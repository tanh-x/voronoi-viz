#include <cmath>
#include "Fortune.hpp"

void beachLineToString(LinkedNode<BeachKey*, BeachValue*>* node, int depth) {
    for (int i = 0; i < depth; i++) std::cout << "|\t";
    if (node == nullptr) {
        std::cout << "--" << std::endl;
        return;
    }

    std::cout << node->key->toString()
              << " // "
              << "P: " << (node->prev ? node->prev->key->toString() : "--")
              << ", N: " << (node->next ? node->next->key->toString() : "--")
              << std::endl;
    if (node->left == nullptr && node->right == nullptr) return;
    beachLineToString(node->left, depth + 1);
    beachLineToString(node->right, depth + 1);
}

void printBeachLineTree(LinkedSplayTree<BeachKey*, BeachValue*, BeachLineComparator> tree) {
    printf("\n\n");
    beachLineToString(tree.root, 0);
}


DCEL* computeVoronoi(const std::vector<Vec2> &sites) {
    auto dcel = new DCELFactory();
    EventComparator eventComp;
    PriorityQueue<Event*, EventComparator> eventQueue(eventComp);
    BeachLineComparator beachLineComp;
    LinkedSplayTree<BeachKey*, BeachValue*, BeachLineComparator> beachLine(beachLineComp);

    for (const Vec2 &site: sites) eventQueue.add(new Event(site));

    double sweepY = eventQueue.peek()->pos.y;

    // Main loop to process all events
    int debugCounter = 0;
    while (!eventQueue.empty()) {
        Event* event = eventQueue.poll();
        sweepY = event->pos.y;
        printf("\n\n-------- Event #%d (%s) --------\n", ++debugCounter, event->isSiteEvent ? "site" : "circle");
        printf("Sweep line position: %f\n", sweepY);
        printf("Starting Beach Line:");
        printBeachLineTree(beachLine);
        if (event->isSiteEvent) {
            processSiteEvent(event, beachLine, eventQueue, dcel, &sweepY);
        } else {
            // TODO: Make this not linear time
            for (auto &v: sites) {
                double radius = event->circleCenter.y - event->pos.y;
                double dist = event->circleCenter.distanceTo(v);
                if (radius - dist > NUMERICAL_TOLERANCE) event->isInvalidated = true;
            }

            if (event->isInvalidated) continue;
            processCircleEvent(event, beachLine, eventQueue, dcel, &sweepY);
        }
//        delete event;
    }

    finalizeEdges(beachLine, dcel);
    return dcel->createDCEL(sites);
}

void processSiteEvent(
    Event* event,
    LinkedSplayTree<BeachKey*, BeachValue*, BeachLineComparator> &beachLine,
    PriorityQueue<Event*, EventComparator> &eventQueue,
    DCELFactory* dcel,
    double* sweepY
) {
    assert(event->isSiteEvent);
    // Extract the site point from the event
    auto* newArc = new BeachKey(sweepY, &event->pos);
    printf("Handling event for %s... ", newArc->toString().c_str());

    // Find the arc directly above the new site point
    LinkedNode<BeachKey*, BeachValue*>* arcAboveNode = beachLine.root;
    while (arcAboveNode) {
        if (arcAboveNode->key->isArc) break;
        // Use the defined natural field ordering
        arcAboveNode = beachLine.compare(arcAboveNode->key, newArc) ? arcAboveNode->right : arcAboveNode->left;
    }

    // If no arc is found directly above, it means this is the first site
    if (!arcAboveNode) {
        assert(beachLine.root == nullptr);
        beachLine.add(newArc, BeachValue::arcPtr(nullptr));
        printf("first arc found, moving on.\n");
        return;  // Early return; no further action needed if this is the first site
    }

    BeachKey* arcAbove = arcAboveNode->key;

    assert(arcAbove->isArc);
    printf("arc above is %s, focus at %s\n", arcAbove->toString().c_str(), arcAbove->focus->toString().c_str());

    // Remove the node
    beachLine.removeNode(arcAboveNode, false);

    // Create new arcs from the split of the old arc
    auto* leftArc = new BeachKey(sweepY, arcAbove->focus);
    auto* rightArc = new BeachKey(sweepY, arcAbove->focus);
    // TODO: For degeneracy, use the left and right arc instead of the same arc.

    // Create two new breakpoints
    auto* leftBreakpoint = new BeachKey(sweepY, leftArc->focus, newArc->focus);
    auto* rightBreakpoint = new BeachKey(sweepY, newArc->focus, rightArc->focus);

    // Add the two breakpoint nodes into the tree
    auto* newEdge = new VertexPair();

    LinkedNode<BeachKey*, BeachValue*>* leftBpNode = beachLine.add(
        leftBreakpoint,
        BeachValue::breakpointPtr(newEdge),
        false
    );
    assert(leftBpNode->parent == nullptr || !leftBpNode->parent->key->isArc);

    LinkedNode<BeachKey*, BeachValue*>* rightBpNode = nullptr;
    bool arcAboveSameLevelDegen = arcAbove->focus->y - event->pos.y < NUMERICAL_TOLERANCE;
    if (!arcAboveSameLevelDegen) {
        rightBpNode = beachLine.add(
            rightBreakpoint,
            BeachValue::breakpointPtr(newEdge),
            false
        );
        assert(rightBpNode->parent == nullptr || !rightBpNode->parent->key->isArc);
    }

    // Add new edge records into the factory
    Vec2 bpProxyOriginVec(
        event->pos.x,
        pointDirectrixParabola(event->pos.x, *arcAbove->focus, *sweepY)
    );

    if (bpProxyOriginVec.isInfinite) {
        assert(arcAboveSameLevelDegen);
        // Degeneracy case: Multiple events at the same x, AND arc above has the same focus.x
        bpProxyOriginVec.x = (newArc->focus->x + arcAbove->focus->x) / 2.0;
    }

    double angle = atan(pointDirectrixGradient(event->pos.x, *arcAbove->focus, *sweepY));
    auto* bpEdgeProxyOrigin = new Vertex(0, bpProxyOriginVec);
    newEdge->offerVertex(bpEdgeProxyOrigin);
    newEdge->angle = angle;
    dcel->offerPair(newEdge);

    LinkedNode<BeachKey*, BeachValue*>* leftArcNode = nullptr;
    LinkedNode<BeachKey*, BeachValue*>* rightArcNode = nullptr;
    if (rightBpNode != nullptr) {
        // Standard case

        // Create three new nodes corresponding to the three arcs
        auto* newArcNode = new LinkedNode<BeachKey*, BeachValue*>(newArc, BeachValue::arcPtr(nullptr));
        leftArcNode = new LinkedNode<BeachKey*, BeachValue*>(leftArc, BeachValue::arcPtr(nullptr));
        rightArcNode = new LinkedNode<BeachKey*, BeachValue*>(rightArc, BeachValue::arcPtr(nullptr));

        // Set up the subtree structure
        assert(leftBpNode->left == nullptr);
        assert(rightBpNode->left == nullptr);
        assert(rightBpNode->right == nullptr);
        leftBpNode->setLeftChild(leftArcNode);
        rightBpNode->setLeftChild(newArcNode);
        rightBpNode->setRightChild(rightArcNode);

        // Handle linked list operations
        leftArcNode->linkPrev(arcAboveNode->prev);
        leftArcNode->linkNext(leftBpNode);
        newArcNode->linkPrev(leftBpNode);
        newArcNode->linkNext(rightBpNode);
        rightArcNode->linkPrev(rightBpNode);
        rightArcNode->linkNext(arcAboveNode->next);

//        beachLine.splay(leftBpNode);
    } else {
        // Degeneracy case: Multiple events at the same x, AND arc above has the same focus.x
        assert(arcAboveSameLevelDegen);

        // Set up the subtree structure
        assert(leftBpNode->left == nullptr);
        // Create three new nodes corresponding to the three arcs
        bool newIsLeft = event->pos.x < arcAbove->focus->x;
        leftArcNode = new LinkedNode<BeachKey*, BeachValue*>(
            newIsLeft ? newArc : leftArc,
            BeachValue::arcPtr(nullptr)
        );
        rightArcNode = new LinkedNode<BeachKey*, BeachValue*>(
            newIsLeft ? rightArc : newArc,
            BeachValue::arcPtr(nullptr)
        );

        leftBpNode->setLeftChild(leftArcNode);
        leftBpNode->setRightChild(rightArcNode);

        // Handle linked list operations
        leftArcNode->linkPrev(arcAboveNode->prev);
        leftArcNode->linkNext(leftBpNode);
        rightArcNode->linkPrev(leftBpNode);
        rightArcNode->linkNext(arcAboveNode->next);
    }

    // The new root should replace the previous arc's node
    assert(arcAboveNode->parent == leftBpNode->parent);

    // Invalidate the split arc's circle event
    if (arcAboveNode->value->circleEvent != nullptr) arcAboveNode->value->circleEvent->isInvalidated = true;

    delete arcAbove;

    // Check for potential circle eventQueue caused by these new arcs
    Event* circEvent1 = checkCircleEvent(leftArcNode, sweepY, eventQueue);
    Event* circEvent2 = checkCircleEvent(rightArcNode, sweepY, eventQueue);
    offerCircleEventPair(eventQueue, circEvent1, circEvent2);
}


Event* checkCircleEvent(
    LinkedNode<BeachKey*, BeachValue*>* arcNode,
    const double* sweepY,
    PriorityQueue<Event*, EventComparator> &eventQueue
) {
    BeachKey* arc = arcNode->key;

    if (!arc->isArc) return nullptr;
    if (arcNode->prev == nullptr /* || arcNode->prev->key->leftSite == nullptr */) return nullptr;
    if (arcNode->next == nullptr /* || arcNode->next->key->rightSite == nullptr */) return nullptr;


    // Extract positions of points
    Vec2 a = *arcNode->prev->key->leftSite;
    Vec2 b = *arc->focus;
    Vec2 c = *arcNode->next->key->rightSite;

    printf("Considering possible circle event of <p%d, p%d, p%d>...\n",
           a.identifier, b.identifier, c.identifier
    );

    if (!(a.identifier != b.identifier && b.identifier != c.identifier && a.identifier != c.identifier)) return nullptr;

    // Check if b is a vertex of a converging circle with a and c
    if (computeDeterminantTest(a, b, c) >= 0) {  // Points must be oriented clockwise
        printf("WARNING: Triplet is not oriented clockwise\n");
    }

    // Calculate the center of the circle through a, b, and c

    Vec2 center = computeCircleCenter(a, b, c);
    if (center.isInfinite) return nullptr;

    double circleEventY = center.y - center.distanceTo(a);

    // Only consider this event if it is below the sweep line
    if (circleEventY + NUMERICAL_TOLERANCE >= *sweepY) {
        printf("Triplet has circumcenter %f above the sweep line, discarding.\n", circleEventY);
        return nullptr;
    }

    // Two way reference between the node and the event
    auto* circleEvent = new Event({center.x, circleEventY}, center, arcNode);
    arcNode->value->circleEvent = circleEvent;

    // Return it to compare with the other one
    return circleEvent;
}


void processCircleEvent(
    Event* event,
    LinkedSplayTree<BeachKey*, BeachValue*, BeachLineComparator> &beachLine,
    PriorityQueue<Event*, EventComparator> &eventQueue,
    DCELFactory* dcel,
    double* sweepY
) {
    if (event->isInvalidated) return;

    // Get arc node corresponding to the circle event
    assert(!event->isSiteEvent);

    LinkedNode<BeachKey*, BeachValue*>* arcNode = event->arcNode;
    assert(arcNode != nullptr);

    BeachKey* arc = arcNode->key;
    assert(arc->isArc);
    assert(arcNode->prev != nullptr);
    assert(arcNode->next != nullptr);

    printf("Handling circle event for arc %s\n", arc->toString().c_str());

    // Add the center of the circle as a new Voronoi vertex
    if (event->circleCenter.isInfinite) return;
    auto* newVoronoiVertex = new Vertex(dcel->numVertices() + 1, event->circleCenter);
    dcel->offerVertex(newVoronoiVertex);

    // Connect breakpoints' edges to it
    LinkedNode<BeachKey*, BeachValue*>* leftBpNode = arcNode->prev;
    LinkedNode<BeachKey*, BeachValue*>* rightBpNode = arcNode->next;
    assert(leftBpNode != nullptr);
    assert(rightBpNode != nullptr);
    leftBpNode->value->breakpointEdge->offerVertex(newVoronoiVertex);
    rightBpNode->value->breakpointEdge->offerVertex(newVoronoiVertex);

    // Then, we need kill the breakpoints and connect the disappearing arc's neighbors in the beach line
    // First, get the nodes to be dissolved and merged
    BeachKey* leftBp = leftBpNode->key;
    BeachKey* rightBp = rightBpNode->key;
    assert(leftBp->rightSite == arc->focus);
    assert(rightBp->leftSite == arc->focus);


    // Create the merged node
    auto* mergedBreakpoint = new BeachKey(sweepY, leftBp->leftSite, rightBp->rightSite);
    auto* mergedBpNode = new LinkedNode<BeachKey*, BeachValue*>(
        mergedBreakpoint,
        BeachValue::breakpointPtr(new VertexPair({newVoronoiVertex, nullptr}))
    );
    double angle = atan(perpendicularBisectorSlope(*leftBp->leftSite, *rightBp->rightSite));
    mergedBpNode->value->breakpointEdge->angle = angle > 0 ? angle - M_PI : angle;
    dcel->offerPair(mergedBpNode->value->breakpointEdge);

    // Handle linked list operations
    assert(leftBpNode->prev->key->isArc);
    assert(leftBpNode->prev->key->isArc);
    mergedBpNode->linkPrev(leftBpNode->prev);
    mergedBpNode->linkNext(rightBpNode->next);

    beachLine.splay(leftBpNode);
    beachLine.splay(rightBpNode);
    // Delete them from the beach line
    LinkedNode<BeachKey*, BeachValue*>* subtreeParent;
    // "<"
    assert(rightBpNode->left == leftBpNode);
    assert(leftBpNode->right == arcNode);
    subtreeParent = rightBpNode->parent;
    mergedBpNode->setLeftChild(leftBpNode->left);
    mergedBpNode->setRightChild(rightBpNode->right);

    // ">"
//    assert(leftBpNode->right == rightBpNode);
//    assert(rightBpNode->left == arcNode);
//    subtreeParent = leftBpNode->parent;
//    mergedBpNode->setLeftChild(leftBpNode->left);
//    mergedBpNode->setRightChild(rightBpNode->right);

    if (subtreeParent == nullptr) beachLine.root = mergedBpNode;
    else if (subtreeParent->left == leftBpNode) subtreeParent->setLeftChild(mergedBpNode);
    else if (subtreeParent->right == leftBpNode) subtreeParent->setRightChild(mergedBpNode);
    else { assert(false); }

    assert(!leftBp->isArc && !rightBp->isArc);

    // Delete affected events
    LinkedNode<BeachKey*, BeachValue*>* prevArcNode = arcNode->prev->prev;
    LinkedNode<BeachKey*, BeachValue*>* nextArcNode = arcNode->next->next;
    assert(prevArcNode != nullptr);
    assert(nextArcNode != nullptr);
    Event* prevCircEvent = prevArcNode->value->circleEvent;
    Event* nextCircEvent = nextArcNode->value->circleEvent;
    if (prevCircEvent != nullptr) prevCircEvent->isInvalidated = true;
    if (nextCircEvent != nullptr) nextCircEvent->isInvalidated = true;

    // Check adjacent arcs for new circle events
    Event* circEvent1 = checkCircleEvent(prevArcNode, sweepY, eventQueue);
    Event* circEvent2 = checkCircleEvent(nextArcNode, sweepY, eventQueue);

    offerCircleEventPair(eventQueue, circEvent1, circEvent2);

    // Clean up the removed arc
    delete arc;
    delete arcNode;
}

void finalizeEdges(
    LinkedSplayTree<BeachKey*, BeachValue*, BeachLineComparator> &beachLine,
    DCELFactory* dcel
) {
    // Close off any unbounded half vertexPairs
}

void offerCircleEventPair(PriorityQueue<Event*, EventComparator> &eventQueue, Event* circEvent1, Event* circEvent2) {
    // Check if the events are null or duplicated
    bool addEvent1;
    bool addEvent2;
    if (circEvent1 != nullptr && circEvent2 != nullptr) {
        addEvent1 = true;
        addEvent2 = circEvent1->pos.x != circEvent2->pos.x
                    || circEvent1->pos.y != circEvent2->pos.y
                    || circEvent1->arcNode != circEvent2->arcNode;
    } else {
        addEvent1 = circEvent1 != nullptr;
        addEvent2 = circEvent2 != nullptr;
    }

    if (addEvent1) {
        assert(circEvent1 != nullptr);
        eventQueue.add(circEvent1);
        printf("Added circle event for arc %s, resolves at %s\n",
               circEvent1->arcNode->key->toString().c_str(),
               circEvent1->pos.toString().c_str()
        );
    }
    if (addEvent2) {
        assert(circEvent2 != nullptr);
        eventQueue.add(circEvent2);
        printf("Added circle event for arc %s, resolves at %s\n",
               circEvent2->arcNode->key->toString().c_str(),
               circEvent2->pos.toString().c_str()
        );
    }
}

double BeachKey::fieldOrdering(double t) const {
    if (isArc) {
        assert(focus);
        assert(leftSite == nullptr);
        assert(rightSite == nullptr);
        return focus->x;
    } else {
        assert(focus == nullptr);
        assert(leftSite);
        assert(rightSite);
        double intersectionX = pointDirectrixIntersectionX(*leftSite, *rightSite, t);
        return (std::isnan(intersectionX) ? (leftSite->x + rightSite->x) / 2.0 : intersectionX);
    }
}

std::string BeachKey::toString() const {
    if (isArc) return "Arc[" + std::to_string(focus->identifier) + "]";
    else return "BP[" + std::to_string(leftSite->identifier) + "," + std::to_string(rightSite->identifier) + "]";
}


BeachValue* BeachValue::breakpointPtr(VertexPair* ptr) {
    return new BeachValue({ptr, nullptr});
}

BeachValue* BeachValue::arcPtr(Event* ptr) {
    return new BeachValue({nullptr, ptr});
}
