#ifndef SHAPE_ATTACK_TRANSFORMATIONS_HPP
#define SHAPE_ATTACK_TRANSFORMATIONS_HPP

#include "linalg/matrix.hpp"

//////////////////////////////////////////////////////////////////////////////

auto grid_tmatt(float alpha, float x, float y) -> linalg::Matrixf<6, 6>
{

    return linalg::Matrixf<6, 6> {{{cosf(alpha), -sinf(alpha), 0, 0, 0, 0},
                                   {sinf(alpha), cosf(alpha), 0, 0, 0, 0},
                                   {x, y, 1, 0, 0, 0},
                                   {0, 0, 0, cosf(alpha), -sinf(alpha), 0},
                                   {0, 0, 0, sinf(alpha), cosf(alpha), 0},
                                   {0, 0, 0, x, y, 1}}};
}

//////////////////////////////////////////////////////////////////////////////

#endif // SHAPE_ATTACK_TRANSFORMATIONS_HPP