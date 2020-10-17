#pragma once
#include "linalg/matrix.hpp"

namespace collision {

template <typename Tp>
auto minkowski_boundary(Tp&& entity, linalg::Vectorf<2> const& origin)
{
    auto points = generate_points(entity);

    float x = origin[0];
    float y = origin[1];
    points[0][0] += x;
    points[0][1] += y;
    points[1][0] += 0.f;
    points[1][1] += y;
    points[2][0] += 0.f;
    points[2][1] += 0.f;
    points[3][0] += x;
    points[3][1] += 0.f;

    float w = points[1][0] - points[0][0];
    float h = points[2][1] - points[0][1];

    return SDL_FRect{
        points[0][0],
        points[0][1],
        w,
        h};
}

}