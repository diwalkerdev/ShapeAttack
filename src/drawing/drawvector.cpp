#include "drawing/drawmatrix.hpp"
#include "linalg/core.hpp"
#include <SDL2/SDL.h>

namespace drawing {

linalg::Matrixf<4, 3> vector_head{{{+0, +0, +1},
                                   {-1, -1, +1},
                                   {-1, +1, +1},
                                   {+0, +0, +1}}};


void draw_vector(SDL_Renderer* renderer, float px, float py, float vx, float vy)
{
    auto arrow = vector_head;
    // TODO use column operator.
    for (auto row : linalg::iter(arrow))
    {
        row[0] *= 10;
        row[1] *= 10;
    }

    float theta = std::atan2(-vy, vx);

    arrow *= rtransf(theta, px + vx, py + vy);

    linalg::Matrixf<2, 3> vector_tail;
    vector_tail[0][0] = arrow[0][0];
    vector_tail[0][1] = arrow[0][1];
    vector_tail[0][2] = arrow[0][2];

    vector_tail[1][0] = px;
    vector_tail[1][1] = py;
    vector_tail[1][2] = 1;

    draw_matrix(renderer, arrow);
    draw_matrix(renderer, vector_tail);
}

}
