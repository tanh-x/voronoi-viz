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
        printf("\n-- Event #%d (%s) --\n Starting Beach Line:", ++debugCounter, event->isSiteEvent ? "site" : "circle");
        printBeachLineTree(beachLine);
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
    LinkedSplayTree<BeachKey*, BeachValue*, BeachLineComparator> &beachLine,
    PriorityQueue<Event*, EventComparator> &eventQueue,
    DCELFactory* dcel,
    double* sweepY
) {
    assert(event->isSiteEvent);
    // Extract the site point from the event
    auto* newArc = new BeachKey(sweepY, &event->pos);

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
        return;  // Early return; no further action needed if this is the first site
    }

    BeachKey* arcAbove = arcAboveNode->key;

    assert(arcAbove->isArc);

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
    LinkedNode<BeachKey*, BeachValue*>* leftBpNode = beachLine.add(
        leftBreakpoint,
        BeachValue::breakpointPtr(new VertexPair()),
        false
    );
    assert(leftBpNode->parent == nullptr || !leftBpNode->parent->key->isArc);

    LinkedNode<BeachKey*, BeachValue*>* rightBpNode = beachLine.add(
        rightBreakpoint,
        BeachValue::breakpointPtr(new VertexPair()),
        false
    );
    assert(rightBpNode->parent == nullptr || !rightBpNode->parent->key->isArc);

    // Create three new nodes corresponding to the three arcs
    auto* newArcNode = new LinkedNode<BeachKey*, BeachValue*>(newArc, BeachValue::arcPtr(nullptr));
    auto* leftArcNode = new LinkedNode<BeachKey*, BeachValue*>(leftArc, BeachValue::arcPtr(nullptr));
    auto* rightArcNode = new LinkedNode<BeachKey*, BeachValue*>(rightArc, BeachValue::arcPtr(nullptr));

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

    // The new root should replace the previous arc's node
    assert(arcAboveNode->parent == leftBpNode->parent);

    // Invalidate the split arc's circle event
    if (arcAboveNode->value->circleEvent != nullptr) arcAboveNode->value->circleEvent->isInvalidated = true;

    delete arcAbove;

    // Check for potential circle eventQueue caused by these new arcs
    checkCircleEvent(leftArcNode, sweepY, eventQueue);
    checkCircleEvent(rightArcNode, sweepY, eventQueue);
}


void checkCircleEvent(
    LinkedNode<BeachKey*, BeachValue*>* arcNode,
    const double* sweepY,
    PriorityQueue<Event*, EventComparator> &eventQueue
) {
    BeachKey* arc = arcNode->key;
    assert(arc->isArc);
    if (arcNode->prev == nullptr /* || arcNode->prev->key->leftSite == nullptr */) return;
    if (arcNode->next == nullptr /* || arcNode->next->key->rightSite == nullptr */) return;

    // Extract positions of points
    Vec2 a = *arcNode->prev->key->leftSite;
    Vec2 b = *arc->focus;
    Vec2 c = *arcNode->next->key->rightSite;

    assert(a.identifier != b.identifier && b.identifier != c.identifier && a.identifier != c.identifier);

    // Check if b is a vertex of a converging circle with a and c
//    assert(computeDeterminantTest(a, b, c) >= 0); // Points must be oriented clockwise

    // Calculate the center of the circle through a, b, and c

    Vec2 center = computeCircleCenter(a, b, c);
    double circleEventY = center.y - center.distanceTo(a);

    // Only consider this event if it is below the sweep line
    if (circleEventY < *sweepY) {
        // Two way reference between the node and the event
        auto* circleEvent = new Event({center.x, circleEventY}, center, arcNode);
        arcNode->value->circleEvent = circleEvent;

        // Add it to the event queue
        eventQueue.add(circleEvent);
    }
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

    // Add the center of the circle as a new Voronoi vertex
    assert(!event->circleCenter.isInfinite);
    auto* newVertex = new Vertex(999, event->circleCenter);
    dcel->vertices.insert(newVertex);

    // Connect breakpoints' edges to it
    arcNode->prev->value->breakpointEdge->offerVertex(newVertex);
    arcNode->next->value->breakpointEdge->offerVertex(newVertex);

    // Then, we need kill the breakpoints and connect the disappearing arc's neighbors in the beach line
    // First, get the nodes to be dissolved and merged
    LinkedNode<BeachKey*, BeachValue*>* leftBpNode = arcNode->prev;
    LinkedNode<BeachKey*, BeachValue*>* rightBpNode = arcNode->next;
    assert(leftBpNode != nullptr);
    assert(rightBpNode != nullptr);

    BeachKey* leftBp = leftBpNode->key;
    BeachKey* rightBp = rightBpNode->key;
    assert(leftBp->rightSite == arc->focus);
    assert(rightBp->leftSite == arc->focus);


    // Create the merged node
    auto* mergedBreakpoint = new BeachKey(sweepY, leftBp->leftSite, rightBp->rightSite);
    auto* mergedBpNode = new LinkedNode<BeachKey*, BeachValue*>(
        mergedBreakpoint,
        BeachValue::breakpointPtr(new VertexPair({newVertex, nullptr}))
    );

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

    // Connect the disappearing arc's neighbors in the beach line
    assert(!leftBp->isArc && !rightBp->isArc);

    // Delete affected events
    LinkedNode<BeachKey*, BeachValue*>* prevArcNode = arcNode->prev->prev;
    if (prevArcNode != nullptr) {
        Event* e = prevArcNode->value->circleEvent;
        if (e != nullptr) e->isInvalidated = true;
        // Check adjacent arcs for additional circle events
        checkCircleEvent(prevArcNode, sweepY, eventQueue);
    }

    LinkedNode<BeachKey*, BeachValue*>* nextArcNode = arcNode->next->next;
    if (nextArcNode != nullptr) {
        Event* e = nextArcNode->value->circleEvent;
        if (e != nullptr) e->isInvalidated = true;
        checkCircleEvent(nextArcNode, sweepY, eventQueue);
    }

    // Clean up the removed arc
//    delete arc;
//    delete arcNode;
}

void finalizeEdges(
    LinkedSplayTree<BeachKey*, BeachValue*, BeachLineComparator> &beachLine,
    DCELFactory* dcel
) {
    // Close off any unbounded half vertexPairs
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
        return pointDirectrixIntersectionX(*leftSite, *rightSite, t);
    }
}


BeachValue* BeachValue::breakpointPtr(VertexPair* ptr) {
    return new BeachValue({ptr, nullptr});
}

BeachValue* BeachValue::arcPtr(Event* ptr) {
    return new BeachValue({nullptr, ptr});
}
