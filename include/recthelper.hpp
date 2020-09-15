#ifndef GAME_RECT_HELPER_H
#define GAME_RECT_HELPER_H

#include "linalg/matrix.hpp"
#include <SDL2/SDL.h>

auto sdl_rect_center(SDL_FRect& rect) -> linalg::Vectorf<2>
{
    return {{(rect.x + rect.w) / 2.f, (rect.y + rect.h) / 2.f}};
}

#endif // GAME_RECT_HELPER_HPP