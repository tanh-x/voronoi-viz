#ifndef VORONOI_VIZ_MATHEMATICS_HPP
#define VORONOI_VIZ_MATHEMATICS_HPP

#include <stdexcept>
#include "utils/math/Vec2.hpp"

#define sq(x) ((x) * (x))

double computeDeterminantTest(const Vec2 &a, const Vec2 &b, const Vec2 &c);

Vec2 computeCircleCenter(const Vec2 &a, const Vec2 &b, const Vec2 &c);

double pointDirectrixIntersectionX(
    const Vec2 &leftParabolaFocus,
    const Vec2 &rightParabolaFocus,
    double directrix
);

Vec2 pointDirectrixIntersectionPos(
    const Vec2 &leftParabolaFocus,
    const Vec2 &rightParabolaFocus,
    double directrix
);

#endif //VORONOI_VIZ_MATHEMATICS_HPP
