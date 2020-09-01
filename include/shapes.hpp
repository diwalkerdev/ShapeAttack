#ifndef SHAPE_ATTACK_SHAPES_HPP
#define SHAPE_ATTACK_SHAPES_HPP

#include <linalg/matrix.hpp>

//////////////////////////////////////////////////////////////////////////////

template <size_t NumSegments>
auto make_circle_points(float radius) -> linalg::Matrixf<NumSegments + 2, 3>
{
    static_assert(NumSegments >= 3);
    constexpr float seg_size = (2 * M_PI / NumSegments);

    linalg::Matrixf<NumSegments + 2, 3> points;

    float theta = 0;

    points[0][0] = 0;
    points[0][1] = 0;
    points[0][2] = 1;

    for (int i = 1; i < NumSegments + 2; ++i)
    {
        points[i][0] = cos(theta) * radius;
        points[i][1] = sin(theta) * radius;
        points[i][2] = 1;
        theta += seg_size;
    }

    return points;
}

//////////////////////////////////////////////////////////////////////////////

template <size_t NumSegments>
struct Shape {
    float radius;
    float x;
    float y;
    float theta;

    linalg::Matrixf<NumSegments + 2, 3> data;
    Shape(float radius, float offset_x = 0, float offset_y = 0)
        : radius(radius)
        , x(offset_x)
        , y(offset_y)
        , theta(0)
        , data(make_circle_points<NumSegments>(radius))
    {
    }
};


struct Bullet {
    bool  is_active;
    float radius;
    float theta;

    linalg::Vectorf<2> position;
    linalg::Vectorf<2> velocity;

    linalg::Matrixf<10 + 2, 3> data;

    Bullet(float radius, float const (&position)[2])
        : is_active(false)
        , radius(radius)
        , theta(0)
        , position{position}
        , velocity{{0, 0}}
        , data(make_circle_points<10>(radius))
    {
    }

    auto fire(float theta)
    {
        this->is_active = true;
        this->theta     = theta;

        auto trajectory = linalg::Vectorf<2>{{cosf(theta), -sinf(theta)}};
        this->velocity  = trajectory * 40;
    }

    Bullet(Bullet const& other) = default;
    Bullet(Bullet&& other)      = default;
    Bullet& operator=(Bullet const& other) = default;
    Bullet& operator=(Bullet&& other) = default;
};

//////////////////////////////////////////////////////////////////////////////

#endif // SHAPE_ATTACK_SHAPES_HPP