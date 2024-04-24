#include <cmath>
#include <string>
#include "utils/math/Vec2.hpp"


Vec2 Vec2::operator+(const Vec2 &other) const {
    return {x + other.x, y + other.y};
}


Vec2 Vec2::operator-(const Vec2 &other) const {
    return {x - other.x, y - other.y};
}


Vec2 Vec2::operator*(double scalar) const {
    return {x * scalar, y * scalar};
}

Vec2 operator*(double scalar, const Vec2 &vec) {
    return {vec.x * scalar, vec.y * scalar};
}

double Vec2::norm() const {
    return sqrt(x * x + y * y);
}

double Vec2::dot(const Vec2 &other) const {
    return x * other.x + y * other.y;
}

double Vec2::cross(const Vec2 &other) const {
    return x * other.y - y * other.x;
}

Vec2 Vec2::normalized() const {
    double k = 1.0 / norm();
    return {x * k, y * k};
}

std::string Vec2::toString() const {
    std::string identifierStr = (
        identifier == VEC2_NO_IDENTIFIER
        ? "" : ("p" + std::to_string(identifier) + " = ")
    );
    return identifierStr + "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}

double Vec2::distanceTo(const Vec2 &other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    return sqrt(dx * dx + dy * dy);
}

Vec2 Vec2::infinity() {
    Vec2 result = Vec2(VEC2_INFTY_PLACEHOLDER, VEC2_INFTY_PLACEHOLDER);
    result.isInfinite = true;
    return result;
}
