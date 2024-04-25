#ifndef VORONOI_VIZ_MATHEMATICS_HPP
#define VORONOI_VIZ_MATHEMATICS_HPP

#include <stdexcept>
#include <limits>
#include "utils/math/Vec2.hpp"

class Vec2;

#define sq(x) ((x) * (x))
#define DOUBLE_INFINITY std::numeric_limits<double>::infinity()
#define QUIET_NAN std::numeric_limits<double>::quiet_NaN()

double computeDeterminantTest(const Vec2 &a, const Vec2 &b, const Vec2 &c);

Vec2 computeCircleCenter(const Vec2 &a, const Vec2 &b, const Vec2 &c);

double pointDirectrixParabola(double x, Vec2 focus, double directrix);

double pointDirectrixGradient(double x, Vec2 focus, double directrix);

double pointDirectrixIntersectionX(
    const Vec2 &leftParabolaFocus,
    const Vec2 &rightParabolaFocus,
    double directrix
);

double perpendicularBisectorSlope(const Vec2 &leftSite, const Vec2 &rightSite);

Vec2 pointDirectrixIntersectionPos(
    const Vec2 &leftParabolaFocus,
    const Vec2 &rightParabolaFocus,
    double directrix
);

#endif //VORONOI_VIZ_MATHEMATICS_HPP
