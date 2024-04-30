#include <cmath>
#include <cassert>
#include "utils/math/mathematics.hpp"
#include "fortune/Fortune.hpp"

double computeDeterminantTest(const Vec2 &a, const Vec2 &b, const Vec2 &c) {
    return (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
}

Vec2 computeCircleCenter(const Vec2 &a, const Vec2 &b, const Vec2 &c) {
    double det = 2 * computeDeterminantTest(a, b, c);

    if (std::abs(det) < NUMERICAL_TOLERANCE) return Vec2::infinity();

    double ux = ((sq(a.x) + sq(a.y)) * (b.y - c.y) +
                 (sq(b.x) + sq(b.y)) * (c.y - a.y) +
                 (sq(c.x) + sq(c.y)) * (a.y - b.y)) / det;
    double uy = ((sq(a.x) + sq(a.y)) * (c.x - b.x) +
                 (sq(b.x) + sq(b.y)) * (a.x - c.x) +
                 (sq(c.x) + sq(c.y)) * (b.x - a.x)) / det;

    return {ux, uy};
}

double pointDirectrixParabola(double x, Vec2 focus, double directrix) {
    double result = (sq(x) - 2 * focus.x * x + sq(focus.x) + sq(focus.y) - sq(directrix))
                    / (2 * (focus.y - directrix));
    assert(!std::isnan(result));
    return result;
}

double pointDirectrixGradient(double x, Vec2 focus, double directrix) {
    double dy = x - focus.x;
    double dx = focus.y - directrix;

    assert(dx != 0 || dy != 0);

    if (dx == 0) return DOUBLE_INFINITY;
    else return dy / dx;
}

double pointDirectrixIntersectionX(const Vec2 &leftParabolaFocus, const Vec2 &rightParabolaFocus, double directrix) {
    double a = leftParabolaFocus.x;
    double b = leftParabolaFocus.y;
    double u = rightParabolaFocus.x;
    double v = rightParabolaFocus.y;
    double d = directrix;

    if (softEquals(b, v)) return (a + u) * 0.5;

    double discriminant = (d - b) * (d - v) * (sq(a - u) + sq(b - v));
    if (discriminant < NUMERICAL_TOLERANCE) return QUIET_NAN;
    return (a * d - a * v + b * u - d * u - sqrt(discriminant)) / (b - v);
}


Vec2 pointDirectrixIntersectionPos(const Vec2 &leftParabolaFocus, const Vec2 &rightParabolaFocus, double directrix) {
    double x = pointDirectrixIntersectionX(leftParabolaFocus, rightParabolaFocus, directrix);
    double leftValue = pointDirectrixParabola(x, leftParabolaFocus, directrix);
    double rightValue = pointDirectrixParabola(x, rightParabolaFocus, directrix);
    assert(leftValue == rightValue);
    return {x, leftValue};
}

double perpendicularBisectorSlope(const Vec2 &leftSite, const Vec2 &rightSite) {
    double dy = rightSite.x - leftSite.x;
    double dx = leftSite.y - rightSite.y;

    assert(dx != 0 || dy != 0);

    if (dx == 0) return DOUBLE_INFINITY;
    else return dy / dx;
}

bool softEquals(double x, double y) {
    return std::abs(x - y) < NUMERICAL_TOLERANCE;
}

bool softEquals(Vec2 v1, Vec2 v2) {
    return std::abs(v1.x - v2.x) < NUMERICAL_TOLERANCE
           && std::abs(v1.y - v2.y) < NUMERICAL_TOLERANCE;
}

std::array<float, 16> orthographicProjection(
    Vec2 bottomLeft,
    Vec2 topRight,
    float near, float far
) {
    std::array<float, 16> mat = {0};

    auto left = static_cast<float>(bottomLeft.x);
    auto right = static_cast<float>(topRight.x);
    auto bottom = static_cast<float>(bottomLeft.y);
    auto top = static_cast<float>(topRight.y);

    mat[0] = 2.0f / (right - left);
    mat[5] = 2.0f / (top - bottom);
    mat[10] = -2.0f / (far - near);
    mat[12] = -(right + left) / (right - left);
    mat[13] = -(top + bottom) / (top - bottom);
    mat[14] = -(far + near) / (far - near);
    mat[15] = 1.0f;

    return mat;
}

