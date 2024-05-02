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

const char* Vec2::toString() const {
    char* result = new char[64];  // %d can only be <11 bytes

//    if (identifier == VEC2_NO_IDENTIFIER) sprintf(result, "(%f, %f)", x, y);
//    else sprintf(result, "%d=(%f, %f)", identifier, x, y);
    sprintf(result, "(%f, %f)", x, y);

    return result;
}

double Vec2::distanceTo(const Vec2 &other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    return sqrt(dx * dx + dy * dy);
}

Vec2 Vec2::infinity() {
    Vec2 result = Vec2(VEC2_PLACEHOLDER, VEC2_PLACEHOLDER);
    result.isInfinite = true;
    return result;
}

Vec2::Vec2(double x, double y) : x(x), y(y) {
    if (x == VEC2_PLACEHOLDER || x == DOUBLE_INFINITY || y == VEC2_PLACEHOLDER || y == DOUBLE_INFINITY) {
        isInfinite = true;
    }
}
