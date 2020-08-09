#ifndef SHAPE_ATTACK_TRANSFORMATIONS_HPP
#define SHAPE_ATTACK_TRANSFORMATIONS_HPP

#include "linalg/matrix.h"

// This version of grid_tmatt preserves the matrix dimension so the resulting
// matrix can still be transformed (shifted).
auto grid_tmatt(float alpha, float x, float y) -> linalg::Matrixf<6, 6>
{
    // clang-format off
    return linalg::Matrixf<6, 6> {{
        cosf(alpha), -sinf(alpha), 0, 0,            0,           0,
        sinf(alpha),  cosf(alpha), 0, 0,            0,           0,
        x,            y,           1, 0,            0,           0,
        0,            0,           0, cosf(alpha), -sinf(alpha), 0,
        0,            0,           0, sinf(alpha),  cosf(alpha), 0,
        0,            0,           0, x,            y,           1
    }};
    // clang-format on
}

// This version of grid_tmat does not preserve the matrix dimensions.
// This is useful for the final transformation as it allows the matrix
// to be directly drawn by the SDL renderer as each pair of value now
// represents a point.
auto grid_tmat(float alpha, float x, float y) -> linalg::Matrixf<6, 4>
{
    // clang-format off
    return linalg::Matrixf<6, 4> {{
        cosf(alpha), -sinf(alpha), 0,            0,
        sinf(alpha),  cosf(alpha), 0,            0,
        x,            y,           0,            0,
        0,            0,           cosf(alpha), -sinf(alpha),
        0,            0,           sinf(alpha),  cosf(alpha),
        0,            0,           x,            y,
    }};
    // clang-format on
}

#endif // SHAPE_ATTACK_TRANSFORMATIONS_HPP