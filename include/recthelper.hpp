#ifndef GAME_RECT_HELPER_H
#define GAME_RECT_HELPER_H

#include "linalg/matrix.hpp"
#include "screen.h"
#include <SDL2/SDL.h>

inline auto rect_center(SDL_FRect const& rect) -> linalg::Vectorf<2>
{
    return {{rect.x + (rect.w / 2.f),
             rect.y + (rect.h / 2.f)}};
}

// TODO: r-l value duplication.
inline auto to_screen_rect(SDL_FRect const& rect)
{
    SDL_FRect dst = rect;

    dst.y = to_screen_y(rect.y) - rect.h;
    return dst;
}

inline auto to_screen_rect(SDL_FRect&& rect)
{
    SDL_FRect dst = rect;

    dst.y = to_screen_y(rect.y) - rect.h;
    return dst;
}

inline auto center_a_in_b(SDL_Rect& a, SDL_Rect const& b)
{
    int x = (b.w - a.w) / 2.f;
    int y = (b.h - a.h) / 2.f;
    a.x   = x;
    a.y   = y;
}

#endif // GAME_RECT_HELPER_HPP
