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
    if (softEquals(v, d)) return u;
    if (softEquals(b, d)) return a;

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

bool softEquals(double x, double y, double tolerance) {
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

    mat[0] = 1;
    mat[5] = 1;
    mat[10] = 1;
    mat[15] = 1;

//    auto left = static_cast<float>(bottomLeft.x);
//    auto right = static_cast<float>(topRight.x);
//    auto bottom = static_cast<float>(bottomLeft.y);
//    auto top = static_cast<float>(topRight.y);
//
//    mat[0] = 2.0f / (right - left);
//    mat[5] = 2.0f / (top - bottom);
//    mat[10] = -2.0f / (far - near);
//    mat[12] = -(right + left) / (right - left);
//    mat[13] = -(top + bottom) / (top - bottom);
//    mat[14] = -(far + near) / (far - near);
//    mat[15] = 1.0f;

    return mat;
}

Vec2 rayIntersectBox(Vec2 pos, double angle, Vec2 bottomLeft, Vec2 topRight) {
    double cosTheta = cos(angle);
    double sinTheta = sin(angle);

    std::vector<Vec2> intersections = {};

    // Check vertical boundaries
    if (cosTheta != 0) {
        double t1 = (bottomLeft.x - pos.x) / cosTheta;
        double y1 = pos.y + t1 * sinTheta;
        if (t1 >= 0 && y1 >= bottomLeft.y && y1 <= topRight.y) {
            intersections.emplace_back(bottomLeft.x, y1);
        }

        double t2 = (topRight.x - pos.x) / cosTheta;
        double y2 = pos.y + t2 * sinTheta;
        if (t2 >= 0 && y2 >= bottomLeft.y && y2 <= topRight.y) {
            intersections.emplace_back(topRight.x, y2);
        }
    }

    // Check horizontal boundaries
    if (sinTheta != 0) {
        double t3 = (bottomLeft.y - pos.y) / sinTheta;
        double x3 = pos.x + t3 * cosTheta;
        if (t3 >= 0 && x3 >= bottomLeft.x && x3 <= topRight.x) {
            intersections.emplace_back(x3, bottomLeft.y);
        }

        double t4 = (topRight.y - pos.y) / sinTheta;
        double x4 = pos.x + t4 * cosTheta;
        if (t4 >= 0 && x4 >= bottomLeft.x && x4 <= topRight.x) {
            intersections.emplace_back(x4, topRight.y);
        }
    }

    // Find the closest intersection point
    Vec2 closest = intersections[0];
    double minT = std::hypot(intersections[0].x - pos.x, intersections[0].y - pos.y);

    for (Vec2 &interxn : intersections) {
        double dist = std::hypot(interxn.x - pos.x, interxn.y - pos.y);
        if (dist < minT) {
            closest = interxn;
            minT = dist;
        }
    }

    return closest;
}

double normalizeRadians(double angle) {
    angle = fmod(angle + M_PI, 2 * M_PI);
    if (angle <= 0) angle += 2 * M_PI;
    return angle - M_PI;
}

double addRadians(double angle1, double angle2) {
    double sum = angle1 + angle2;
    return normalizeRadians(sum);
}


