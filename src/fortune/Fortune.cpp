#include <cmath>
#include <cassert>
#include "fortune/Fortune.hpp"


FortuneSweeper::FortuneSweeper(const std::vector<Vec2> &sites) : sites(sites) {
    EventComparator eventComp;
    ChainComparator chainComp;
    this->eventQueue = new PriorityQueue<Event*, EventComparator>(eventComp);
    this->beachLine = new LinkedSplayTree<Chain*, TreeValueFacade*, ChainComparator>(chainComp);
    this->factory = new DCELFactory();

    // Populate the event queue with site events
    for (const Vec2 &site: sites) eventQueue->add(new Event(site));
    this->sweepY = eventQueue->peek()->y();
}


void FortuneSweeper::stepNextEvent() {
    if (eventQueue->empty()) throw std::out_of_range("Event queue is empty; all events already handled.");


    Event* event = eventQueue->poll();
    currentEventCounter++;
    sweepY = event->y();

    printf("\n-------- ");
    printf("Event #%d (%s)", currentEventCounter, event->isSiteEvent ? "site" : "circle");
    printf(" --------\n");
    printf("Sweep line position: %f\n", sweepY);
    printf("Starting Beach Line:");
    printBeachLine();

    if (event->isSiteEvent) handleSiteEvent(event);
    else {
        // TODO: Make this not linear time
        for (auto &v: sites) {
            double radius = event->circleCenter.y - event->pos.y;
            double dist = event->circleCenter.distanceTo(v);
            if (radius - dist > NUMERICAL_TOLERANCE) event->isInvalidated = true;
        }
        handleCircleEvent(event);
    }

    lastHandledEvent = event;
}


DCEL* FortuneSweeper::finalize() {
    return factory->createDCEL(sites);
}

DCEL* FortuneSweeper::computeAll() {
    while (!eventQueue->empty()) stepNextEvent();

    return finalize();
}

void FortuneSweeper::handleSiteEvent(Event* event) {
    assert(event->isSiteEvent);
    // Extract the site point from the event
    auto* newArc = new Chain(&sweepY, &event->pos);
    printf("Handling event for %s... ", newArc->toString());

    // Find the arc directly above the new site point
    LinkedNode<Chain*, TreeValueFacade*>* arcAboveNode = beachLine->root;
    while (arcAboveNode) {
        if (arcAboveNode->key->isArc) break;
        // Use the defined natural field ordering
        arcAboveNode = beachLine->compare(arcAboveNode->key, newArc) ? arcAboveNode->right : arcAboveNode->left;
    }

    // If no arc is found directly above, it means this is the first site
    if (!arcAboveNode) {
        assert(beachLine->root == nullptr);
        beachLine->add(newArc, TreeValueFacade::arcPtr(nullptr));
        printf("first arc found, moving on.\n");
        return;  // Early return; no further action needed if this is the first site
    }

    Chain* arcAbove = arcAboveNode->key;

    assert(arcAbove->isArc);
    printf("arc above is %s, focus at %s\n", arcAbove->toString(), arcAbove->focus->toString());

    // Remove the node
    beachLine->removeNode(arcAboveNode, false);

    // Create new arcs from the split of the old arc
    auto* leftArc = new Chain(&sweepY, arcAbove->focus);
    auto* rightArc = new Chain(&sweepY, arcAbove->focus);
    // TODO: For degeneracy, use the left and right arc instead of the same arc.

    // Create two new breakpoints
    auto* leftBreakpoint = new Chain(&sweepY, leftArc->focus, newArc->focus);
    auto* rightBreakpoint = new Chain(&sweepY, newArc->focus, rightArc->focus);

    // Add the two breakpoint nodes into the tree
    auto* newEdge = new VertexPair();

    LinkedNode<Chain*, TreeValueFacade*>* leftBpNode = beachLine->add(
        leftBreakpoint,
        TreeValueFacade::breakpointPtr(newEdge),
        false
    );
    assert(leftBpNode->parent == nullptr || !leftBpNode->parent->key->isArc);

    LinkedNode<Chain*, TreeValueFacade*>* rightBpNode = nullptr;
    bool arcAboveSameLevelDegen = softEquals(arcAbove->focus->y, event->pos.y);
    if (!arcAboveSameLevelDegen) {
        rightBpNode = beachLine->add(
            rightBreakpoint,
            TreeValueFacade::breakpointPtr(newEdge),
            false
        );
        assert(rightBpNode->parent == nullptr || !rightBpNode->parent->key->isArc);
    }

    // Add new edge records into the factory
    Vec2 bpProxyOriginVec(
        event->pos.x,
        pointDirectrixParabola(event->pos.x, *arcAbove->focus, sweepY)
    );

    if (bpProxyOriginVec.isInfinite) {
        assert(arcAboveSameLevelDegen);
        // Degeneracy case: Multiple events at the same x, AND arc above has the same focus.x
        bpProxyOriginVec.x = (newArc->focus->x + arcAbove->focus->x) / 2.0;
    }

    double angle = atan(pointDirectrixGradient(event->pos.x, *arcAbove->focus, sweepY));
    auto* bpEdgeProxyOrigin = new Vertex(0, bpProxyOriginVec);
    newEdge->offerVertex(bpEdgeProxyOrigin);
    newEdge->angle = angle;
    factory->offerPair(newEdge);

    LinkedNode<Chain*, TreeValueFacade*>* leftArcNode;
    LinkedNode<Chain*, TreeValueFacade*>* rightArcNode;
    if (rightBpNode != nullptr) {
        // Standard case

        // Create three new nodes corresponding to the three arcs
        auto* newArcNode = new LinkedNode<Chain*, TreeValueFacade*>(newArc, TreeValueFacade::arcPtr(nullptr));
        leftArcNode = new LinkedNode<Chain*, TreeValueFacade*>(leftArc, TreeValueFacade::arcPtr(nullptr));
        rightArcNode = new LinkedNode<Chain*, TreeValueFacade*>(rightArc, TreeValueFacade::arcPtr(nullptr));

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

//        beachLine->splay(leftBpNode);
    } else {
        // Degeneracy case: Multiple events at the same x, AND arc above has the same focus.x
        assert(arcAboveSameLevelDegen);

        // Set up the subtree structure
        assert(leftBpNode->left == nullptr);
        // Create three new nodes corresponding to the three arcs
        bool newIsLeft = event->pos.x < arcAbove->focus->x;
        leftArcNode = new LinkedNode<Chain*, TreeValueFacade*>(
            newIsLeft ? newArc : leftArc,
            TreeValueFacade::arcPtr(nullptr)
        );
        rightArcNode = new LinkedNode<Chain*, TreeValueFacade*>(
            newIsLeft ? rightArc : newArc,
            TreeValueFacade::arcPtr(nullptr)
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
    Event* circEvent1 = checkAndCreateCircleEvent(leftArcNode);
    Event* circEvent2 = checkAndCreateCircleEvent(rightArcNode);
    offerCircleEventPair(circEvent1, circEvent2);
}

void FortuneSweeper::handleCircleEvent(Event* event) {
    if (event->isInvalidated) return;

    // Get arc node corresponding to the circle event
    assert(!event->isSiteEvent);

    LinkedNode<Chain*, TreeValueFacade*>* arcNode = event->arcNode;
    assert(arcNode != nullptr);

    Chain* arc = arcNode->key;
    assert(arc->isArc);
    assert(arcNode->prev != nullptr);
    assert(arcNode->next != nullptr);

    printf("Handling circle event for arc %s\n", arc->toString());

    bool cocircular = softEquals(event->circleCenter, lastHandledEvent->circleCenter);

    if (!cocircular) {  // Standard case
        // Add the center of the circle as a new Voronoi vertex
        if (event->circleCenter.isInfinite) return;
        auto* newVoronoiVertex = new Vertex(factory->numVertices() + 1, event->circleCenter);
        factory->offerVertex(newVoronoiVertex);

        // Connect breakpoints' edges to it
        LinkedNode<Chain*, TreeValueFacade*>* leftBpNode = arcNode->prev;
        LinkedNode<Chain*, TreeValueFacade*>* rightBpNode = arcNode->next;
        assert(leftBpNode != nullptr);
        assert(rightBpNode != nullptr);
        leftBpNode->value->breakpointEdge->offerVertex(newVoronoiVertex);
        rightBpNode->value->breakpointEdge->offerVertex(newVoronoiVertex);

        // Then, we need kill the breakpoints and connect the disappearing arc's neighbors in the beach line
        // First, get the nodes to be dissolved and merged
        Chain* leftBp = leftBpNode->key;
        Chain* rightBp = rightBpNode->key;
        assert(leftBp->rightSite == arc->focus);
        assert(rightBp->leftSite == arc->focus);

        // Create the merged node
        auto* mergedBreakpoint = new Chain(&sweepY, leftBp->leftSite, rightBp->rightSite);
        auto* mergedBpNode = new LinkedNode<Chain*, TreeValueFacade*>(
            mergedBreakpoint,
            TreeValueFacade::breakpointPtr(new VertexPair({newVoronoiVertex, nullptr}))
        );
        double angle = atan(perpendicularBisectorSlope(*leftBp->leftSite, *rightBp->rightSite));
        mergedBpNode->value->breakpointEdge->angle = angle > 0 ? angle - M_PI : angle;
        factory->offerPair(mergedBpNode->value->breakpointEdge);

        // Handle linked list operations
        assert(leftBpNode->prev->key->isArc);
        assert(leftBpNode->prev->key->isArc);
        mergedBpNode->linkPrev(leftBpNode->prev);
        mergedBpNode->linkNext(rightBpNode->next);

        beachLine->splay(leftBpNode);
        beachLine->splay(rightBpNode);
        // Delete them from the beach line
        LinkedNode<Chain*, TreeValueFacade*>* subtreeParent;
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

        if (subtreeParent == nullptr) beachLine->root = mergedBpNode;
        else if (subtreeParent->left == leftBpNode) subtreeParent->setLeftChild(mergedBpNode);
        else if (subtreeParent->right == leftBpNode) subtreeParent->setRightChild(mergedBpNode);
        else { assert(false); }

        assert(!leftBp->isArc && !rightBp->isArc);
    } else {
        // Degeneracy: more than 3 cocircular sites
        assert(softEquals(event->pos, lastHandledEvent->pos));


        assert(false);
    }

    // Delete affected events
    LinkedNode<Chain*, TreeValueFacade*>* prevArcNode = arcNode->prev->prev;
    LinkedNode<Chain*, TreeValueFacade*>* nextArcNode = arcNode->next->next;
    assert(prevArcNode != nullptr);
    assert(nextArcNode != nullptr);
    Event* prevCircEvent = prevArcNode->value->circleEvent;
    Event* nextCircEvent = nextArcNode->value->circleEvent;
    if (prevCircEvent != nullptr) prevCircEvent->isInvalidated = true;
    if (nextCircEvent != nullptr) nextCircEvent->isInvalidated = true;

    // Check adjacent arcs for new circle events
    Event* circEvent1 = checkAndCreateCircleEvent(prevArcNode);
    Event* circEvent2 = checkAndCreateCircleEvent(nextArcNode);

    offerCircleEventPair(circEvent1, circEvent2);

    // Clean up the removed arc
    delete arc;
    delete arcNode;
}

Event* FortuneSweeper::checkAndCreateCircleEvent(LinkedNode<Chain*, TreeValueFacade*>* arcNode) const {
    Chain* arc = arcNode->key;

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
    if (circleEventY - NUMERICAL_TOLERANCE > sweepY) {
        printf("Triplet has circumcenter %f above the sweep line, discarding.\n", circleEventY);
        return nullptr;
    }

    // Two-way reference between the node and the event
    auto* circleEvent = new Event({center.x, circleEventY}, center, arcNode);
    arcNode->value->circleEvent = circleEvent;

    // Return it to compare with the other one
    return circleEvent;
}


void FortuneSweeper::offerCircleEventPair(Event* circEvent1, Event* circEvent2) {
    // Check if the events are null or duplicated
    bool addEvent1;
    bool addEvent2;
    if (circEvent1 != nullptr && circEvent2 != nullptr) {
        addEvent1 = true;
        addEvent2 = !softEquals(circEvent1->pos.x, circEvent2->pos.x)
                    || !softEquals(circEvent1->pos.y, circEvent2->pos.y)
                    || circEvent1->arcNode->key->focus != circEvent2->arcNode->key->focus;
    } else {
        addEvent1 = circEvent1 != nullptr;
        addEvent2 = circEvent2 != nullptr;
    }

    if (addEvent1) {
        assert(circEvent1 != nullptr);
        eventQueue->add(circEvent1);
        printf("Added circle event for arc %s, resolves at %s\n",
               circEvent1->arcNode->key->toString(),
               circEvent1->pos.toString()
        );
    }
    if (addEvent2) {
        assert(circEvent2 != nullptr);
        eventQueue->add(circEvent2);
        printf("Added circle event for arc %s, resolves at %s\n",
               circEvent2->arcNode->key->toString(),
               circEvent2->pos.toString()
        );
    }
}

void FortuneSweeper::printBeachLine() {
    printf("\n\n");
    beachLineToString(beachLine->root, 0);
}

void FortuneSweeper::beachLineToString(LinkedNode<Chain*, TreeValueFacade*>* node, int depth) {
    for (int i = 0; i < depth; i++) printf("|\t");
    if (node == nullptr) {
        printf("--");
        return;
    }

    printf(
        "%s // P: %s, N: %s\n",
        node->key->toString(),
        node->prev ? node->prev->key->toString() : "--",
        node->next ? node->next->key->toString() : "--"
    );

    if (node->left == nullptr && node->right == nullptr) return;
    beachLineToString(node->left, depth + 1);
    beachLineToString(node->right, depth + 1);
}
