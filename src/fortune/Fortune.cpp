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
        // Check if it exactly coincides with the arc above
        if (softEquals(event->pos.x, arcAboveNode->key->fieldOrdering(sweepY))) {
            assert(!arcAboveNode->key->isArc);
            handleSiteAtBottomDegen(event);
            return;
        }
        // Use the defined natural field ordering
        arcAboveNode = beachLine->compare(arcAboveNode->key, newArc) ? arcAboveNode->rightChild
                                                                     : arcAboveNode->leftChild;
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
        assert(leftBpNode->leftChild == nullptr);
        assert(rightBpNode->leftChild == nullptr);
        assert(rightBpNode->rightChild == nullptr);
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
        assert(leftBpNode->leftChild == nullptr);
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

    delete arcAbove;

    // Check for potential circle eventQueue caused by these new arcs
    Event* circEvent1 = checkAndCreateCircleEvent(leftArcNode);
    Event* circEvent2 = checkAndCreateCircleEvent(rightArcNode);
    offerCircleEventPair(circEvent1, circEvent2);
}

void FortuneSweeper::handleCircleEvent(Event* event) {
    if (event->isInvalidated) {
        printf("Event has already been invalidated, exiting\n");
        return;
    }

    // Sanity checks
    assert(!event->isSiteEvent);

    LinkedNode<Chain*, TreeValueFacade*>* arcNode = event->arcNode;
    assert(arcNode != nullptr);

    // Get arc node corresponding to the circle event
    Chain* arc = arcNode->key;

    // More sanity checks
    assert(arc->isArc);
    assert(arcNode->prev != nullptr);
    assert(arcNode->next != nullptr);

    printf("Handling circle event for %s\n", arc->toString());

    // Temporarily holders for left and right breakpoints, preparing for "expansion" in the next while loop
    LinkedNode<Chain*, TreeValueFacade*>* leftBpNode = arcNode->prev;
    LinkedNode<Chain*, TreeValueFacade*>* rightBpNode = arcNode->next;
    printf("Checking possible cocircular events\n");

    // Grab every circle event that also occurs here
    // Only break when we've gathered every "cocircular event"
    // They are contiguous in the heap, because the comparator puts all site events before circular events
    // if they have the same x and y coordinates (within a small numerical tolerance)
    bool cocircular = false;  // For assertions only
    while (true) {
        printf(
            "Left BP: %s. Right BP: %s. Checking next event...\n",
            leftBpNode->key->toString(), rightBpNode->key->toString()
        );

        // Check if this is the last event in the queue
        if (eventQueue->empty()) break;

        // Get the next event without pulling it out of the heap
        Event* nextEvent = eventQueue->peek();

        // Check if its a circle event
        if (nextEvent->isSiteEvent) break;

        // Check if it occurs at the same coordinates
        assert(nextEvent->y() <= event->y());  // No tolerance in this assertion
        if (!softEquals(nextEvent->pos, event->pos)) break;

        // If so, we found a cocircular event
        cocircular = true;

        // Then, the circle center should also occur at the same spot
        assert(softEquals(nextEvent->circleCenter, event->circleCenter));

        // Extract it from the heap
        Event* polledEvent = eventQueue->poll();
        assert(polledEvent == nextEvent);

        // Get its left and right breakpoints
        LinkedNode<Chain*, TreeValueFacade*>* nextEventLeftBpNode = nextEvent->arcNode->prev;
        LinkedNode<Chain*, TreeValueFacade*>* nextEventRightBpNode = nextEvent->arcNode->next;

        if (beachLine->compare(nextEventLeftBpNode->key, leftBpNode->key)) {
            leftBpNode = nextEventLeftBpNode;
        }

        if (beachLine->compare(rightBpNode->key, nextEventRightBpNode->key)) {
            rightBpNode = nextEventRightBpNode;
        }
    }
    assert(leftBpNode != nullptr);
    assert(rightBpNode != nullptr);
    assert(leftBpNode != rightBpNode);
    printf("No further cocircular events found, moving on to gathering vanishing arcs...\n");

    // Get the left and right breakpoints bounding merge
    Chain* leftBp = leftBpNode->key;
    Chain* rightBp = rightBpNode->key;

    // Sanity checks
    if (cocircular) {
        assert(leftBpNode->next->next != rightBpNode);
    } else {
        assert(leftBpNode->next->next == rightBpNode);
        assert(leftBp->rightSite == arc->focus);
        assert(rightBp->leftSite == arc->focus);
    }

    // Get all vanishing chains, from left to right
    std::vector<LinkedNode<Chain*, TreeValueFacade*>*> vanishingArcNodes;
    std::vector<LinkedNode<Chain*, TreeValueFacade*>*> vanishingBpNodes;
    // Start the scan
    LinkedNode<Chain*, TreeValueFacade*>* node = leftBpNode;
    vanishingBpNodes.push_back(leftBpNode);
    while (node != rightBpNode) {
        // Node currently has a Chain of a breakpoint as its key
        assert(!node->key->isArc);

        // The next arc will disappear after the circle event
        node = node->next;
        assert(node->key->isArc);
        vanishingArcNodes.push_back(node);

        // It is not possible for the next node to be nullptr, as it must not be the rightmost arc,
        // as otherwise we forgot to escape the loop when we should've
        assert(node->next != nullptr);

        // Get the right-side breakpoint
        node = node->next;
        assert(!node->key->isArc);

        // This breakpoint will be vanishing after the event (or merged if its the right breakpoint)
        vanishingBpNodes.push_back(node);
    }

    // Quick chain type checks
    for (auto &an: vanishingArcNodes) assert(an->key->isArc);
    for (auto &bn: vanishingBpNodes) assert(!bn->key->isArc);
    // Check for cocircularity
    int numVanishingArcs = static_cast<int>(vanishingArcNodes.size());
    int numMergingBreakpoints = static_cast<int>(vanishingBpNodes.size());
    assert((!cocircular && numMergingBreakpoints == 2) || (cocircular && numMergingBreakpoints >= 3));
    assert((!cocircular && numVanishingArcs == 1) || (cocircular && numVanishingArcs >= 2));
    // Furthermore, the first and last element are leftBpNode and rightBpNode
    assert(vanishingBpNodes[0] == leftBpNode);
    assert(vanishingBpNodes[numMergingBreakpoints - 1] == rightBpNode);

    // Add the center of the circle as a new Voronoi vertex
    if (event->circleCenter.isInfinite) return;
    auto* newVoronoiVertex = new Vertex(factory->numVertices() + 1, event->circleCenter);
    factory->offerVertex(newVoronoiVertex);

    // Connect every merging breakpoints' edges to it
    for (auto &bn: vanishingBpNodes) {
        VertexPair* breakpointEdge = bn->value->breakpointEdge;
        breakpointEdge->offerVertex(newVoronoiVertex);
    }

    // Then, we need kill the breakpoints and connect the disappearing arc's neighbors in the beach line

    // First, create the merged node
    auto* mergedBreakpoint = new Chain(&sweepY, leftBp->leftSite, rightBp->rightSite);
    auto* mergedBpNode = new LinkedNode<Chain*, TreeValueFacade*>(
        mergedBreakpoint,
        TreeValueFacade::breakpointPtr(new VertexPair({newVoronoiVertex, nullptr}))
    );
    double angle = atan(perpendicularBisectorSlope(*leftBp->leftSite, *rightBp->rightSite));
    mergedBpNode->value->breakpointEdge->angle = angle > 0 ? angle - M_PI : angle;
    factory->offerPair(mergedBpNode->value->breakpointEdge);

    // Handle linked list operations for the new merged breakpoint node
    assert(leftBpNode->prev->key->isArc);
    assert(leftBpNode->prev->key->isArc);
    mergedBpNode->linkPrev(leftBpNode->prev);
    mergedBpNode->linkNext(rightBpNode->next);

    // Delete every node that DOESN'T belong to one of the two bounding breakpoints
    // Start with leaves
    for (auto &an: vanishingArcNodes) {
        assert(an->leftChild == nullptr && an->rightChild == nullptr);
        beachLine->removeNode(an, false);
    }
    // Delete every breakpoint that DOESN't belong to one of the two bounding breakpoints
    for (int i = 1; i <= numMergingBreakpoints - 2; i++) {
        auto bn = vanishingBpNodes[i];
        beachLine->removeNode(bn, false);
    }

    beachLine->splay(leftBpNode);
    beachLine->splay(rightBpNode);
    // Delete them from the beach line
    LinkedNode<Chain*, TreeValueFacade*>* subtreeParent;
    // "<" (when not circular)
    assert(rightBpNode->leftChild == leftBpNode);
    assert(leftBpNode->rightChild == nullptr);
    subtreeParent = rightBpNode->parent;
    mergedBpNode->setLeftChild(leftBpNode->leftChild);
    mergedBpNode->setRightChild(rightBpNode->rightChild);

    // ">"
    //    assert(leftBpNode->right == rightBpNode);
    //    assert(rightBpNode->left == arcNode);
    //    subtreeParent = leftBpNode->parent;
    //    mergedBpNode->setLeftChild(leftBpNode->left);
    //    mergedBpNode->setRightChild(rightBpNode->right);

    if (subtreeParent == nullptr) beachLine->root = mergedBpNode;
    else if (subtreeParent->leftChild == leftBpNode) subtreeParent->setLeftChild(mergedBpNode);
    else if (subtreeParent->rightChild == leftBpNode) subtreeParent->setRightChild(mergedBpNode);
    else { assert(false); }

    assert(!leftBp->isArc && !rightBp->isArc);

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
//    delete arc;
//    delete arcNode;
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
    double radius = center.distanceTo(a);
    double circleEventY = center.y - radius;

    // Only consider this event if it is below the sweep line
    if (circleEventY + NUMERICAL_TOLERANCE > sweepY) {
        printf("Triplet has circumcenter %f above the sweep line, discarding.\n", circleEventY);
        return nullptr;
    }

    // Check if the arc itself already has a circle event
    Event* prevCircleEvent = arcNode->value->circleEvent;
    if (prevCircleEvent != nullptr) {
        printf("Arc has previous circle event that resolves at (%f, %f)\n", prevCircleEvent->x(),
               prevCircleEvent->y());
        assert(!prevCircleEvent->isSiteEvent);
        if (prevCircleEvent->y() < circleEventY) return nullptr;
        else prevCircleEvent->isInvalidated = true;
    }

    // Two-way reference between the node and the event
    auto* circleEvent = new Event({center.x, circleEventY}, center, arcNode);
    for (auto &v: sites) {
        double dist = center.distanceTo(v);
        if (radius - dist > NUMERICAL_TOLERANCE) return nullptr;
    }
    arcNode->value->circleEvent = circleEvent;

    // Return it to compare with the other one
    return circleEvent;
}


void FortuneSweeper::handleSiteAtBottomDegen(Event* event) {
    printf("Degeneracy: site below breakpoint (%s), exiting", event->arcNode->key->toString());
    assert(false);
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
        printf("Added circle event for %s, resolves at %s\n",
               circEvent1->arcNode->key->toString(),
               circEvent1->pos.toString()
        );
    }
    if (addEvent2) {
        assert(circEvent2 != nullptr);
        eventQueue->add(circEvent2);
        printf("Added circle event for %s, resolves at %s\n",
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
        printf("--\n");
        return;
    }

    printf(
        "%s // P: %s, N: %s\n",
        node->key->toString(),
        node->prev ? node->prev->key->toString() : "--",
        node->next ? node->next->key->toString() : "--"
    );

    if (node->leftChild == nullptr && node->rightChild == nullptr) return;
    beachLineToString(node->leftChild, depth + 1);
    beachLineToString(node->rightChild, depth + 1);
}
