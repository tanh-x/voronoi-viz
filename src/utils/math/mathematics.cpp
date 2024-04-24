#include <cmath>
#include <cassert>
#include "utils/math/mathematics.hpp"

double computeDeterminantTest(const Vec2 &a, const Vec2 &b, const Vec2 &c) {
    return (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
}

Vec2 computeCircleCenter(const Vec2 &a, const Vec2 &b, const Vec2 &c) {
    double det = 2 * computeDeterminantTest(a, b, c);

    if (det == 0) return Vec2::infinity();

    double ux = ((sq(a.x) + sq(a.y)) * (b.y - c.y) +
                 (sq(b.x) + sq(b.y)) * (c.y - a.y) +
                 (sq(c.x) + sq(c.y)) * (a.y - b.y)) / det;
    double uy = ((sq(a.x) + sq(a.y)) * (c.x - b.x) +
                 (sq(b.x) + sq(b.y)) * (a.x - c.x) +
                 (sq(c.x) + sq(c.y)) * (b.x - a.x)) / det;

    return {ux, uy};
}

double pointDirectrixParabola(double x, Vec2 focus, double directrix) {
    return (sq(x) - 2 * focus.x * x + sq(focus.x) + sq(focus.y) - sq(directrix))
           / (2 * (focus.y - directrix));
}

double pointDirectrixIntersectionX(const Vec2 &leftParabolaFocus, const Vec2 &rightParabolaFocus, double directrix) {
    double a = leftParabolaFocus.x;
    double b = leftParabolaFocus.y;
    double u = rightParabolaFocus.x;
    double v = rightParabolaFocus.y;
    double d = directrix;

    double discriminant = (d - b) * (d - v) * (sq(a - u) + sq(b - v));
    return (a * d - a * v + b * u - d * u + sqrt(discriminant)) / (b - v);
}


Vec2 pointDirectrixIntersectionPos(const Vec2 &leftParabolaFocus, const Vec2 &rightParabolaFocus, double directrix) {
    double x = pointDirectrixIntersectionX(leftParabolaFocus, rightParabolaFocus, directrix);
    double leftValue = pointDirectrixParabola(x, leftParabolaFocus, directrix);
    double rightValue = pointDirectrixParabola(x, rightParabolaFocus, directrix);
    assert(leftValue == rightValue);
    return {x, leftValue};
}