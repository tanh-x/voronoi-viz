#include <cassert>
#include <cmath>
#include "fortune/BeachChain.hpp"


double BeachChain::fieldOrdering(double t) const {
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

const char* BeachChain::toString() const {
    char* result = new char[64];  // %d can only be <11 bytes

    if (isArc) sprintf(result, "Arc[%d]", focus->identifier);
    else sprintf(result, "BP[%d,%d]", leftSite->identifier, rightSite->identifier);

    return result;
}


TreeValueFacade* TreeValueFacade::breakpointPtr(VertexPair* ptr) {
    return new TreeValueFacade({ptr, nullptr});
}

TreeValueFacade* TreeValueFacade::arcPtr(Event* ptr) {
    return new TreeValueFacade({nullptr, ptr});
}

bool ChainComparator::operator()(BeachChain* a, BeachChain* b) const {
    assert(a->sweepY == b->sweepY);
    double orderingParameter = *a->sweepY;
    return a->fieldOrdering(orderingParameter) < b->fieldOrdering(orderingParameter);
}
