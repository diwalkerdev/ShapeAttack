#pragma once
#include "linalg/core.hpp"
#include "misc.hpp"
#include "typedefs.h"
#include <SDL2/SDL.h>

namespace animation {

///////////////////////////////////////////////////////////////////////////////

template <int Frames>
struct LRUPTextureMapDescriptor {
    SDL_Texture const* const texture;

    const int texture_width;
    const int texture_height;

    const int frame_width;
    const int frame_height;

    const int frames{Frames};
};

enum class Direction : int8 {
    LEFT,
    RIGHT,
    UP,
    DOWN
};

///////////////////////////////////////////////////////////////////////////////

template <int Frames>
auto make_LRUPDescriptor(SDL_Texture* texture)
{
    uint32 format;
    int    access, w, h;
    SDL_QueryTexture(texture, &format, &access, &w, &h);

    return LRUPTextureMapDescriptor<Frames>{texture, w, h, w / 2, h / 4};
}

///////////////////////////////////////////////////////////////////////////////

template <int Frames>
SDL_Rect get_frame_rect(LRUPTextureMapDescriptor<Frames>& desc, Direction direction, int frame)
{
    assert(frame < Frames);

    int x = frame * desc.frame_width;
    int y = static_cast<int>(direction) * desc.frame_height;

    return SDL_Rect{x, y, desc.frame_width, desc.frame_height};
}

///////////////////////////////////////////////////////////////////////////////

constexpr static const float SEGS[4]{
    45.f * M_PI / 180.f,
    45.f * 3 * M_PI / 180.f,
    45.f * 5 * M_PI / 180.f,
    45.f * 7 * M_PI / 180.f};

auto is_in_segment(int seg, float theta)
{
    return (theta >= SEGS[seg] && theta < SEGS[seg + 1]);
}

///////////////////////////////////////////////////////////////////////////////

auto determine_direction(float theta)
{
    Direction direction;

    // Remember because of the screen to euclidian transformation rotations go clockwise.
    if (is_in_segment(0, theta))
    {
        direction = Direction::DOWN;
    }
    else if (is_in_segment(1, theta))
    {
        direction = Direction::LEFT;
    }
    else if (is_in_segment(2, theta))
    {
        direction = Direction::UP;
    }
    else
    {
        direction = Direction::RIGHT;
    }
    return direction;
}

///////////////////////////////////////////////////////////////////////////////

template <int Frames>
auto animate(LRUPTextureMapDescriptor<Frames>& desc, linalg::Vectorf<2> const& vel, Direction& direction, int& accumilator, int& frame)
{
    auto theta = std::atan2(-vel[1], vel[0]);
    auto mag   = linalg::magnitude(vel);

    // This transforms theta from -pi -> pi to 0 -> 2*pi/
    if (theta < 0)
    {
        theta += 2 * M_PI;
    }

    if (mag < 20)
    {
        frame = 0;
        // leave the direction as it is to leave the player facing in the direction of travel as they stop.
    }
    else
    {
        direction = determine_direction(theta);
        accumilator += (mag > 50) ? 1 : -1;

        if (accumilator < 0)
        {
            accumilator = 0;
        }

        if (accumilator == 30)
        {
            accumilator = 0;
            frame += 1;
            if (frame >= Frames)
            {
                frame = 0;
            }
        }
    }

    return get_frame_rect(desc, direction, frame);
}

///////////////////////////////////////////////////////////////////////////////

}
