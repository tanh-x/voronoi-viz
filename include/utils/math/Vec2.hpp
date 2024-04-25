#ifndef VORONOI_VIZ_VEC2_HPP
#define VORONOI_VIZ_VEC2_HPP

#include "mathematics.hpp"

#define VEC2_NO_IDENTIFIER (-12345)

#define VEC2_PLACEHOLDER (1.234567e8f)

class Vec2 {
public:
    double x;
    double y;
    int identifier {VEC2_NO_IDENTIFIER};
    bool isInfinite = false;

    Vec2(double x, double y);

    Vec2(double x, double y, int id) : x(x), y(y), identifier(id) {}

    [[nodiscard]] double norm() const;

    [[nodiscard]] double dot(const Vec2 &other) const;

    [[nodiscard]] double cross(const Vec2 &other) const;

    [[nodiscard]] Vec2 operator+(const Vec2 &other) const;

    [[nodiscard]] Vec2 operator-(const Vec2 &other) const;

    [[nodiscard]] Vec2 operator*(double scalar) const;

    [[nodiscard]] double distanceTo(const Vec2 &other) const;

    friend Vec2 operator*(double scalar, const Vec2 &vec);

    [[nodiscard]] Vec2 normalized() const;

    [[nodiscard]] std::string toString() const;

    static Vec2 infinity();
};

#endif //VORONOI_VIZ_VEC2_HPP
