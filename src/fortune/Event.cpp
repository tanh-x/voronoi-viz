#include "fortune/Event.hpp"

double Event::x() const {
    return pos.x;
}

double Event::y() const {
    return pos.y;
}

bool EventComparator::operator()(Event* a, Event* b) const {
    if (std::abs(a->pos.y - b->pos.y) < NUMERICAL_TOLERANCE) return a->pos.x < b->pos.x;
    return a->pos.y > b->pos.y;
}
