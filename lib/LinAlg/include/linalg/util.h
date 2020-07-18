#ifndef LINALG_UTIL_H
#define LINALG_UTIL_H

#include "linalg/matrix.h"
#include <cmath>


linalg::Matrix<float, 2, 2> lrotzf(float alpha)
{
    return {{
        cosf(alpha), -sinf(alpha), 
        sinf(alpha), cosf(alpha)
    }};
}


linalg::Matrix<float, 2, 2> rrotzf(float alpha)
{
    return {{
        cosf(alpha), sinf(alpha), 
        -sinf(alpha), cosf(alpha)
    }};
}

#endif  // LINALG_UTIL_H