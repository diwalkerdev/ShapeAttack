#ifndef GAME_RECT_HELPER_H
#define GAME_RECT_HELPER_H

#include "linalg/matrix.hpp"
#include "screen.h"
#include <SDL2/SDL.h>

auto sdl_rect_center(SDL_FRect& rect) -> linalg::Vectorf<2>
{
    return {{rect.x + (rect.w / 2.f),
             rect.y + (rect.h / 2.f)}};
}

// TODO: r-l value duplication.
auto to_screen_rect(SDL_FRect& rect)
{
    SDL_FRect dst = rect;

    dst.y = to_screen_y(rect.y) - rect.h;
    return dst;
}

auto to_screen_rect(SDL_FRect&& rect)
{
    SDL_FRect dst = rect;

    dst.y = to_screen_y(rect.y) - rect.h;
    return dst;
}
#endif // GAME_RECT_HELPER_HPP