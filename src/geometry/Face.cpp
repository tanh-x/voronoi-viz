#include "geometry/Face.hpp"

std::string Face::toString() const {
    return "f" + std::to_string(label);
}
