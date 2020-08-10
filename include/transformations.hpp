#ifndef SHAPE_ATTACK_TRANSFORMATIONS_HPP
#define SHAPE_ATTACK_TRANSFORMATIONS_HPP

#include "linalg/matrix.h"

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

#endif // SHAPE_ATTACK_TRANSFORMATIONS_HPP