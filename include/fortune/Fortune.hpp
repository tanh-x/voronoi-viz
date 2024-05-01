#ifndef FORTUNE_HPP
#define FORTUNE_HPP

#include <vector>
#include "utils/math/Vec2.hpp"
#include "utils/PriorityQueue.hpp"
#include "utils/LinkedSplayTree.hpp"
#include "Event.hpp"
#include "BeachChain.hpp"
#include "geometry/DCEL.hpp"

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

    void handleCircleEvent(Event* event);

    Event* checkAndCreateCircleEvent(LinkedNode<BeachChain*, TreeValueFacade*>* arcNode) const;

    void offerCircleEventPair(Event* circEvent1, Event* circEvent2);

    void printBeachLine();

    void beachLineToString(LinkedNode<BeachChain*, TreeValueFacade*>* node, int depth);

    void handleSiteAtBottomDegen(Event* event);


};

#endif
