#pragma once
#include <SDL2/SDL.h>

namespace collision {

inline auto is_point_in_rect(float x, float y, SDL_FRect const& rect)
{
    bool in_x = (x > rect.x) && (x < (rect.x + rect.w));
    bool in_y = (y > rect.y) && (y < (rect.y + rect.h));

    bool result{in_x && in_y};
    return result;
}

}