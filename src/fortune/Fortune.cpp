#include <cmath>
#include <cassert>
#include "fortune/Fortune.hpp"


FortuneSweeper::FortuneSweeper(const std::vector<Vec2> &sites) : sites(sites) {
    EventComparator eventComp;
    ChainComparator chainComp;
    this->eventQueue = new PriorityQueue<Event*, EventComparator>(eventComp);
    this->beachLine = new LinkedSplayTree<BeachChain*, TreeValueFacade*, ChainComparator>(chainComp);
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
    else handleCircleEvent(event);

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
    auto* newArc = new BeachChain(&sweepY, &event->pos);
    printf("Handling event for %s... ", newArc->toString());

    // Find the arc directly above the new site point
    LinkedNode<BeachChain*, TreeValueFacade*>* arcAboveNode = beachLine->root;
    while (arcAboveNode) {
        if (arcAboveNode->key->isArc) break;
        // Check if it exactly coincides with the arc above
        if (softEquals(event->pos.x, arcAboveNode->key->fieldOrdering(sweepY))) {
            assert(!arcAboveNode->key->isArc);

            // Also if there's a circle event here as well
            if (!eventQueue->empty()
                && !eventQueue->peek()->isSiteEvent
                && softEquals(eventQueue->peek()->pos, event->pos)) {
                printf("WARNING: Site below breakpoint, coinciding with a (co)circular event!.\n");
            }
            handleSiteAtBottomDegen(event, newArc, arcAboveNode);
            return;
        }
        // Use the defined natural field ordering
        arcAboveNode = beachLine->compare(arcAboveNode->key, newArc) ? arcAboveNode->rightChild
                                                                     : arcAboveNode->leftChild;
    }

    // If no arc is found directly above, it means this is the first site
    if (!arcAboveNode) {
        assert(beachLine->root == nullptr);
        beachLine->add(newArc, TreeValueFacade::arcPtr());
        printf("first arc found, moving on.\n");
        return;  // Early return; no further action needed if this is the first site
    }

    BeachChain* arcAbove = arcAboveNode->key;

    assert(arcAbove->isArc);
    printf("arc above is %s, focus at %s\n", arcAbove->toString(), arcAbove->focus->toString());

    // Remove the node
    beachLine->removeNode(arcAboveNode, false);

    // Create new arcs from the split of the old arc
    auto* leftArc = new BeachChain(&sweepY, arcAbove->focus);
    auto* rightArc = new BeachChain(&sweepY, arcAbove->focus);
    // TODO: For degeneracy, use the left and right arc instead of the same arc.

    // Create two new breakpoints
    auto* leftBreakpoint = new BeachChain(&sweepY, leftArc->focus, newArc->focus);
    auto* rightBreakpoint = new BeachChain(&sweepY, newArc->focus, rightArc->focus);

    // Add the two breakpoint nodes into the tree
    auto* newEdge = new VertexPair();

    LinkedNode<BeachChain*, TreeValueFacade*>* leftBpNode = beachLine->add(
        leftBreakpoint,
        TreeValueFacade::breakpointPtr(newEdge),
        false
    );
    assert(leftBpNode->parent == nullptr || !leftBpNode->parent->key->isArc);

    LinkedNode<BeachChain*, TreeValueFacade*>* rightBpNode = nullptr;
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

    LinkedNode<BeachChain*, TreeValueFacade*>* leftArcNode;
    LinkedNode<BeachChain*, TreeValueFacade*>* rightArcNode;
    if (rightBpNode != nullptr) {
        // Standard case

        // Create three new nodes corresponding to the three arcs
        auto* newArcNode = new LinkedNode<BeachChain*, TreeValueFacade*>(newArc, TreeValueFacade::arcPtr());
        leftArcNode = new LinkedNode<BeachChain*, TreeValueFacade*>(leftArc, TreeValueFacade::arcPtr());
        rightArcNode = new LinkedNode<BeachChain*, TreeValueFacade*>(rightArc, TreeValueFacade::arcPtr());

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
        leftArcNode = new LinkedNode<BeachChain*, TreeValueFacade*>(
            newIsLeft ? newArc : leftArc,
            TreeValueFacade::arcPtr()
        );
        rightArcNode = new LinkedNode<BeachChain*, TreeValueFacade*>(
            newIsLeft ? rightArc : newArc,
            TreeValueFacade::arcPtr()
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

void FortuneSweeper::handleCircleEvent(Event* event, bool skipEdgeCreation) {
    if (event->isInvalidated) {
        printf("Event has already been invalidated, exiting\n");
        return;
    }

    // Sanity checks
    assert(!event->isSiteEvent);

    LinkedNode<BeachChain*, TreeValueFacade*>* arcNode = event->arcNode;
    assert(arcNode != nullptr);

    // Get arc node corresponding to the circle event
    BeachChain* arc = arcNode->key;

    // More sanity checks
    assert(arc->isArc);
    assert(arcNode->prev != nullptr);
    assert(arcNode->next != nullptr);

    printf("Handling circle event for %s\n", arc->toString());

    // Henceforth, arcs will be referred to as "vanishing" if they will disappear after this (co)circle event
    // These arcs are bounded on two sides by two breakpoints, which will eventually be found and assigned to
    // leftMerger and rightMerger, referred to as "merging" breakpoints. Every other breakpoint involved in
    // the event that will disappear are also called "vanishing breakpoints".

    VanishingChains vanishing = getVanishingChains(event);

    LinkedNode<BeachChain*, TreeValueFacade*>* leftMerger = vanishing.leftMerger;
    LinkedNode<BeachChain*, TreeValueFacade*>* rightMerger = vanishing.rightMerger;
    std::vector<LinkedNode<BeachChain*, TreeValueFacade*>*> vanishingArcNodes = *vanishing.vanishingArcNodes;
    std::vector<LinkedNode<BeachChain*, TreeValueFacade*>*> vanishingBpNodes = *vanishing.vanishingBpNodes;

    // Everything should be fine here

    // Add the center of the circle as a new Voronoi vertex
    if (event->circleCenter.isInfinite) return;
    auto* newVoronoiVertex = new Vertex(factory->numVertices() + 1, event->circleCenter);
    factory->offerVertex(newVoronoiVertex);

    // Connect every merging breakpoints' edges to it
    std::vector<LinkedNode<BeachChain*, TreeValueFacade*>*> breakpoints(vanishingBpNodes);
    breakpoints.push_back(leftMerger);
    breakpoints.push_back(rightMerger);
    for (auto &bn: breakpoints) {
        VertexPair* breakpointEdge = bn->value->breakpointEdge;

        if (breakpointEdge == nullptr) {
            // This means this breakpoint did not come from a standard site event nor a circle event,
            // but rather a handleSiteAtBottomDegen case. We add a new edge for it here.
            double angle = atan(perpendicularBisectorSlope(*bn->key->leftSite, *bn->key->rightSite));

            auto* newEdge = new VertexPair();
            newEdge->offerVertex(newVoronoiVertex);

            // Always point downwards
            newEdge->angle = angle > 0 ? angle - M_PI : angle;

            // Add it back into the value pointer
            bn->value->breakpointEdge = newEdge;
            factory->offerPair(newEdge);
        } else {
            breakpointEdge->offerVertex(newVoronoiVertex);
        }
    }

    if (!skipEdgeCreation) {
        // Get the left and right breakpoints bounding merge
        BeachChain* leftBp = leftMerger->key;
        BeachChain* rightBp = rightMerger->key;

        // Then, we need kill the breakpoints and connect the disappearing arc's neighbors in the beach line

        // First, create the merged node
        auto* mergedBreakpoint = new BeachChain(&sweepY, leftBp->leftSite, rightBp->rightSite);
        auto* mergedBpNode = new LinkedNode<BeachChain*, TreeValueFacade*>(
            mergedBreakpoint,
            TreeValueFacade::breakpointPtr(new VertexPair({newVoronoiVertex, nullptr}))
        );
        double angle = atan(perpendicularBisectorSlope(*leftBp->leftSite, *rightBp->rightSite));
        mergedBpNode->value->breakpointEdge->angle = angle > 0 ? angle - M_PI : angle;
        factory->offerPair(mergedBpNode->value->breakpointEdge);

        // Handle linked list operations for the new merged breakpoint node
        mergedBpNode->linkPrev(leftMerger->prev);
        mergedBpNode->linkNext(rightMerger->next);

        // Delete every node involved in the event EXCEPT for the two merging breakpoints, which is removed later
        // Start with leaves, which are vanishing arcs
        for (auto &an: vanishingArcNodes) beachLine->removeNode(an, false);

        // Then, delete every vanishing breakpoint
        for (auto &bn: vanishingBpNodes) beachLine->removeNode(bn, false);

        // Pull the two merging breakpoints to be near the root
        beachLine->splay(leftMerger);
        beachLine->splay(rightMerger);
        // Delete them from the beach line
        LinkedNode<BeachChain*, TreeValueFacade*>* subtreeParent;
        // "<" (when not circular)
        assert(rightMerger->leftChild == leftMerger);
        assert(leftMerger->rightChild == nullptr);
        subtreeParent = rightMerger->parent;
        mergedBpNode->setLeftChild(leftMerger->leftChild);
        mergedBpNode->setRightChild(rightMerger->rightChild);

        // ">"
        //    assert(leftMerger->right == rightMerger);
        //    assert(rightMerger->left == arcNode);
        //    subtreeParent = leftMerger->parent;
        //    mergedBpNode->setLeftChild(leftMerger->left);
        //    mergedBpNode->setRightChild(rightMerger->right);

        if (subtreeParent == nullptr) beachLine->root = mergedBpNode;
        else if (subtreeParent->leftChild == leftMerger) subtreeParent->setLeftChild(mergedBpNode);
        else if (subtreeParent->rightChild == leftMerger) subtreeParent->setRightChild(mergedBpNode);
        else { assert(false); }

        assert(!leftBp->isArc && !rightBp->isArc);
    }

    // Delete affected events
    LinkedNode<BeachChain*, TreeValueFacade*>* prevArcNode = arcNode->prev->prev;
    LinkedNode<BeachChain*, TreeValueFacade*>* nextArcNode = arcNode->next->next;
    assert(prevArcNode != nullptr);
    assert(nextArcNode != nullptr);
    Event* prevCircEvent = prevArcNode->value->circleEvent;
    Event* nextCircEvent = nextArcNode->value->circleEvent;
    if (prevCircEvent != nullptr) prevCircEvent->isInvalidated = true;
    if (nextCircEvent != nullptr) nextCircEvent->isInvalidated = true;

    // As well as every event belonging to the vanishing arcs
    for (auto &an: vanishingArcNodes) {
        if (an->value->circleEvent == nullptr) continue;
        an->value->circleEvent->isInvalidated = true;
    }


    // Check adjacent arcs for new circle events
    Event* circEvent1 = checkAndCreateCircleEvent(prevArcNode);
    Event* circEvent2 = checkAndCreateCircleEvent(nextArcNode);

    offerCircleEventPair(circEvent1, circEvent2);

    // Clean up the removed arc
//    delete arc;
//    delete arcNode;
}

Event* FortuneSweeper::checkAndCreateCircleEvent(LinkedNode<BeachChain*, TreeValueFacade*>* arcNode) const {
    BeachChain* arc = arcNode->key;

    if (!arc->isArc) return nullptr;
    if (arcNode->prev == nullptr /* || arcNode->prev->key->leftSite == nullptr */) return nullptr;
    if (arcNode->next == nullptr /* || arcNode->next->key->rightSite == nullptr */) return nullptr;


    // Extract positions of foci
    Vec2 a = *arcNode->prev->key->leftSite;
    Vec2 b = *arc->focus;
    Vec2 c = *arcNode->next->key->rightSite;


    printf("Considering possible circle event of <p%d, p%d, p%d>...\n",
           a.identifier, b.identifier, c.identifier
    );

    if (!(a.identifier != b.identifier && b.identifier != c.identifier && a.identifier != c.identifier)) return nullptr;

    // Check if b is a vertex of a converging circle with a and c
    if (computeDeterminantTest(a, b, c) >= 0) {  // Points must be oriented clockwise
        printf("Triplet is not oriented clockwise, discarding.\n");
        return nullptr;
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


void FortuneSweeper::handleSiteAtBottomDegen(
    Event* event,
    BeachChain* newArc,
    LinkedNode<BeachChain*, TreeValueFacade*>* bpAboveNode
) {
    printf(
        "\nDegeneracy: site %s below breakpoint %s.\n",
        event->pos.toString(),
        bpAboveNode->key->toString()
    );

    // Check for (co)circular events also happening here

    // Pointer to the two adjacent arcs
    auto* leftArcNode = bpAboveNode->prev;
    auto* rightArcNode = bpAboveNode->next;

    // First, bring the breakpoint node to the root
    beachLine->splay(bpAboveNode);
    assert(beachLine->root == bpAboveNode);
    assert(bpAboveNode->parent == nullptr);

    // The two adjacent arcs are the on the ends of the left and right subtree
    assert(bpAboveNode->leftChild != nullptr);
    assert(bpAboveNode->rightChild != nullptr);
    auto* leftSubtree = bpAboveNode->leftChild;
    auto* rightSubtree = bpAboveNode->rightChild;

    // Assert to see if they're the correct subtrees
    assert(leftSubtree->rightmost() == leftArcNode);
    assert(rightSubtree->leftmost() == rightArcNode);

    // Make a proxy node for the "pseudo" circle event
    auto* newArcNode = new LinkedNode<BeachChain*, TreeValueFacade*>(
        newArc,
        TreeValueFacade::arcPtr()
    );

    // Create two new breakpoints
    auto* leftBreakpoint = new BeachChain(&sweepY, leftArcNode->key->focus, newArc->focus);
    auto* rightBreakpoint = new BeachChain(&sweepY, newArc->focus, rightArcNode->key->focus);
    auto* leftBpNode = new LinkedNode<BeachChain*, TreeValueFacade*>(
        leftBreakpoint,
        TreeValueFacade::breakpointPtr()
    );
    auto* rightBpNode = new LinkedNode<BeachChain*, TreeValueFacade*>(
        rightBreakpoint,
        TreeValueFacade::breakpointPtr()
    );

    // Linked list operations
    leftArcNode->linkNext(leftBpNode);
    newArcNode->linkPrev(leftBpNode);
    newArcNode->linkNext(rightBpNode);
    rightArcNode->linkPrev(rightBpNode);

    // Replace the tree structure
    beachLine->root = leftBpNode;
    leftBpNode->setLeftChild(leftSubtree);
    leftBpNode->setRightChild(rightBpNode);
    rightBpNode->setLeftChild(newArcNode);
    rightBpNode->setRightChild(rightSubtree);
    // Hopefully that went well

    // Create a new circle event and add it to the queue, resolving immediately
    Vec2 a = *leftArcNode->key->focus;
    Vec2 b = *newArc->focus;
    Vec2 c = *rightArcNode->key->focus;

    // Calculate the center of the circle through a, b, and c
    Vec2 center = computeCircleCenter(a, b, c);

    // Impossible, since already below the breakpoint,
    // and there's an ordering of x in EventComparator
    assert(!center.isInfinite);

    double radius = center.distanceTo(a);
    double circleEventY = center.y - radius;

    // We should be exactly on the sweep line
    assert(softEquals(circleEventY, sweepY));

    // Two-way reference between the node and the event
    auto* circleEvent = new Event({center.x, circleEventY}, center, newArcNode);
    newArcNode->value->circleEvent = circleEvent;

    // Finally, resolve the event immediately
    eventQueue->add(circleEvent);
    printf("Handled degenerate site event, current beach line:");
    printBeachLine();
    printf("Added pseudo-circle event to queue, resolving now...\n\n");

    handleCircleEvent(circleEvent, true);

    // Additionally, end the breakpoint node's edge, since we will be removing all references to it from the tree
    // Retrieve the new voronoi vertex
    Vertex* newVoronoiVertex = leftBpNode->value->breakpointEdge->v1;
    assert(rightBpNode->value->breakpointEdge->v1 == newVoronoiVertex);
    bpAboveNode->value->breakpointEdge->offerVertex(newVoronoiVertex);
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

void FortuneSweeper::beachLineToString(LinkedNode<BeachChain*, TreeValueFacade*>* node, int depth) {
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

VanishingChains FortuneSweeper::getVanishingChains(Event* event) {
    assert(!event->isSiteEvent);

    LinkedNode<BeachChain*, TreeValueFacade*>* arcNode = event->arcNode;
    BeachChain* arc = arcNode->key;
    bool cocircular = false;

    // Temporarily holders for left and right breakpoints, preparing for the traversal in the next while loop
    LinkedNode<BeachChain*, TreeValueFacade*>* leftMerger = arcNode->prev;
    LinkedNode<BeachChain*, TreeValueFacade*>* rightMerger = arcNode->next;
    printf("Checking possible cocircular sites\n");

    // Grab every circle event that also occurs here
    // Traverse left and right of the current chain to find all vanishing/merging arcs/breakpoints
    auto* vanishingArcNodes = new std::vector<LinkedNode<BeachChain*, TreeValueFacade*>*>();
    auto* vanishingBpNodes = new std::vector<LinkedNode<BeachChain*, TreeValueFacade*>*>();
    vanishingArcNodes->push_back(arcNode);
    // Traverse the chains left until we hit the merging breakpoint
    while (true) {
        // brain note: VGX
        // This node currently has a breakpoint as its key
        assert(!leftMerger->key->isArc);
        // Left-side chain cannot be null, since an arc that is in a circle event must not be on the side
        assert(leftMerger->prev != nullptr);

        leftMerger = leftMerger->prev;  // brain note: NBP
        // Now, we're looking at a node that has an arc as its key
        assert(leftMerger->key->isArc);

        // Check if it has a circle event, and the event coincides with this one
        if (leftMerger->value->circleEvent == nullptr ||
            !softEquals(leftMerger->value->circleEvent->pos, event->pos)) {
            // If not, then we have traversed the arc left of the merging breakpoint
            // Go back to the last breakpoint to get the left-side merging breakpoint
            leftMerger = leftMerger->next;
            break;
        }
        // Otherwise, this arc has a focus that is cocircular with respect to this event
        cocircular = true;

        // Add the arc and its right-side breakpoint to the list of vanishing events
        vanishingBpNodes->push_back(leftMerger->next);
        vanishingArcNodes->push_back(leftMerger);

        printf(
            "Left-side: Found cocircular site at %s, whose arc is %s.\n",
            leftMerger->key->focus->toString(), leftMerger->key->toString()
        );

        leftMerger = leftMerger->prev;
    }

    // Same as above, but march to the right side until we hit the merging breakpoint
    while (true) {
        assert(!rightMerger->key->isArc);
        assert(rightMerger->prev != nullptr);

        rightMerger = rightMerger->next;
        assert(rightMerger->key->isArc);

        if (rightMerger->value->circleEvent == nullptr ||
            !softEquals(rightMerger->value->circleEvent->pos, event->pos)) {
            rightMerger = rightMerger->prev;
            break;
        }
        cocircular = true;

        vanishingBpNodes->push_back(rightMerger->prev);
        vanishingArcNodes->push_back(rightMerger);
        printf(
            "Right-side: Found cocircular site at %s, whose arc is %s.\n",
            rightMerger->key->focus->toString(), rightMerger->key->toString()
        );

        rightMerger = rightMerger->next;
    }


    // Get the left and right breakpoints bounding merge
    BeachChain* leftBp = leftMerger->key;
    BeachChain* rightBp = rightMerger->key;

    // We're going to carry out a bunch of assertions and sanity checks next
    assert(leftMerger != rightMerger);
    assert(leftMerger->prev->key->isArc);
    assert(rightMerger->next->key->isArc);

    // Chain type assertions
    for (auto &an: *vanishingArcNodes) assert(an->key->isArc);
    for (auto &bn: *vanishingBpNodes) assert(!bn->key->isArc);

    // Assertions for cocircularity
    if (cocircular) {
        assert(leftMerger->next->next != rightMerger);
    } else {
        assert(leftMerger->next->next == rightMerger);
        assert(leftBp->rightSite == arc->focus);
        assert(rightBp->leftSite == arc->focus);
    }
    int numVanishingArcs = static_cast<int>(vanishingArcNodes->size());
    int numVanishingBreakpoints = static_cast<int>(vanishingBpNodes->size());
    assert((!cocircular && numVanishingBreakpoints == 0) || (cocircular && numVanishingBreakpoints >= 1));
    assert((!cocircular && numVanishingArcs == 1) || (cocircular && numVanishingArcs >= 2));

    return {
        leftMerger,
        rightMerger,
        vanishingArcNodes,
        vanishingBpNodes,
        cocircular
    };
}

