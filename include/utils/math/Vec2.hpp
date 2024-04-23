#ifndef VORONOI_VIZ_VEC2_HPP
#define VORONOI_VIZ_VEC2_HPP

class Vec2 {
public:
    double x;
    double y;

    Vec2(double x, double y) : x(x), y(y) {}

    [[nodiscard]] double norm() const;

    [[nodiscard]] double dot(const Vec2 &other) const;

    [[nodiscard]] double cross(const Vec2 &other) const;

    [[nodiscard]] Vec2 operator+(const Vec2 &other) const;

    [[nodiscard]] Vec2 operator-(const Vec2 &other) const;

    [[nodiscard]] Vec2 operator*(double scalar) const;

    friend Vec2 operator*(double scalar, const Vec2 &vec);

    [[nodiscard]] Vec2 normalized() const;
};

#endif //VORONOI_VIZ_VEC2_HPP
