#ifndef FORTUNE_HPP
#define FORTUNE_HPP

#include <vector>
#include "utils/math/Vec2.hpp"
#include "utils/PriorityQueue.hpp"
#include "utils/LinkedSplayTree.hpp"
#include "Event.hpp"
#include "BeachChain.hpp"
#include "geometry/DCEL.hpp"

typedef struct VanishingChains {
    LinkedNode<BeachChain*, TreeValueFacade*>* leftMerger;
    LinkedNode<BeachChain*, TreeValueFacade*>* rightMerger;
    std::vector<LinkedNode<BeachChain*, TreeValueFacade*>*>* vanishingArcNodes;
    std::vector<LinkedNode<BeachChain*, TreeValueFacade*>*>* vanishingBpNodes;
    bool cocircular;
} VanishingChains;

class FortuneSweeper {
public:
    const std::vector<Vec2> &sites;

    double sweepY;
    int currentEventCounter = 0;

    explicit FortuneSweeper(const std::vector<Vec2> &sites);

    void stepNextEvent();

    DCEL* computeAll();

    DCEL* finalize();

private:
    PriorityQueue<Event*, EventComparator>* eventQueue;
    LinkedSplayTree<BeachChain*, TreeValueFacade*, ChainComparator>* beachLine;
    DCELFactory* factory;

    Event* lastHandledEvent {nullptr};

    void handleSiteEvent(Event* event);

    // Returns the breakpoint running from the new Voronoi vertex
    LinkedNode<BeachChain*, TreeValueFacade*>*  handleCircleEvent(Event* event, bool skipEdgeCreation = false);

    Event* checkAndCreateCircleEvent(LinkedNode<BeachChain*, TreeValueFacade*>* arcNode) const;

    void offerCircleEventPair(Event* circEvent1, Event* circEvent2);

    void printBeachLine();

    void beachLineToString(LinkedNode<BeachChain*, TreeValueFacade*>* node, int depth);

    static VanishingChains getVanishingChains(
        LinkedNode<BeachChain*, TreeValueFacade*>* arcNode,
        Vec2 eventPosition
    );

    void handleSiteAtBottomDegen(
        Event* event,
        BeachChain* newArc,
        LinkedNode<BeachChain*, TreeValueFacade*>* bpAboveNode
    );
};


#endif
