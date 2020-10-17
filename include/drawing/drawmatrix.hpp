#ifndef UNTITLED_DRAWING_DRAWMATRIX_HPP
#define UNTITLED_DRAWING_DRAWMATRIX_HPP

#include "linalg/matrix.hpp"
#include "linalg/trans.hpp"
#include "screen.h"
#include <SDL2/SDL.h>

namespace drawing {

template <typename Tp, std::size_t M, std::size_t N>
void draw_matrix(SDL_Renderer* renderer, linalg::Matrix<Tp, M, N>& mat)
{
    for (auto i : irange<M - 1>())
    {
        SDL_RenderDrawLineF(renderer,
                            mat[i][0],
                            to_screen_y(mat[i][1]),
                            mat[i + 1][0],
                            to_screen_y(mat[i + 1][1]));
    }
}

}

#endif // UNTITLED_DRAWING_DRAWMATRIX_HPP