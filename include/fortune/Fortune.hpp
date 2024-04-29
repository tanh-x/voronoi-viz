#ifndef FORTUNE_HPP
#define FORTUNE_HPP

#include <vector>
#include "utils/math/Vec2.hpp"
#include "utils/PriorityQueue.hpp"
#include "utils/LinkedSplayTree.hpp"
#include "Event.hpp"
#include "Chain.hpp"
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
    LinkedSplayTree<Chain*, TreeValueFacade*, ChainComparator>* beachLine;
    DCELFactory* factory;

    void handleSiteEvent(Event* event);

    void handleCircleEvent(Event* event);

    Event* checkAndCreateCircleEvent(LinkedNode<Chain*, TreeValueFacade*>* arcNode) const;

    void offerCircleEventPair(Event* circEvent1, Event* circEvent2);

    void printBeachLine();

    void beachLineToString(LinkedNode<Chain*, TreeValueFacade*>* node, int depth);

};

#endif
