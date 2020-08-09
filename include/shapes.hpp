#ifndef SHAPE_ATTACK_SHAPES_HPP
#define SHAPE_ATTACK_SHAPES_HPP

#include <linalg/matrix.h>

template <typename Scalar, size_t NumSegments>
linalg::Matrix<Scalar, NumSegments + 2, 3> make_circle_points(float radius)
{
    static_assert(NumSegments >= 3);
    constexpr float seg_size = (2 * M_PI / NumSegments);

    linalg::Matrix<Scalar, NumSegments + 2, 3> points;

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


template <typename T, int NUM_POINTS>
auto make_circle(T circumference)
{
    return make_circle_points<T, NUM_POINTS>(circumference / 2);
}

template <typename T>
auto make_square(T width)
{
    return make_circle_points<T, 4>(width / 2);
}

template <typename T>
auto make_triangle(T length)
{
    return make_circle_points<T, 3>(length / 2);
}


#endif // SHAPE_ATTACK_SHAPES_HPP