#ifndef GAME_ENTITY_HPP
#define GAME_ENTITY_HPP

#include "linalg/matrix.hpp"
#include "recthelper.hpp"
#include "screen.h"
#include <SDL2/SDL.h>

struct Entity {
    SDL_Texture*       texture;
    linalg::Vectorf<2> dim;

    // State space representation.
    linalg::Matrixf<2, 2> X; // position, velocity.
    linalg::Matrixf<2, 2> Xdot; // velocity, acceleration.
    linalg::Matrixf<2, 2> Y;

    linalg::Matrixf<2, 2> A;
    linalg::Matrixf<2, 2> B;

    float imass;
    float k; // friction.

    void update(float dt, linalg::Matrixf<2, 2> const& u)
    {
        Xdot = (X + (dt * A * X)) + ((dt * B) * u);
        Y    = X;
        X    = Xdot;
    }
};

struct EntityStatic {
    SDL_Texture* texture;
    SDL_FRect    r;
};

//////////////////////////////////////////////////////////////////////////////


auto sdl_rect(Entity& entity)
{
    SDL_FRect rect;

    rect.x = entity.X[0][0];
    rect.y = to_screen_y(entity.X[0][1]) - entity.dim[1];
    rect.w = entity.dim[0];
    rect.h = entity.dim[1];

    return rect;
}

auto sdl_rect(EntityStatic& entity)
{
    return entity.r;
}

auto sdl_rect_center(Entity& entity)
{
    auto r = sdl_rect(entity);
    return sdl_rect_center(r);
}

//////////////////////////////////////////////////////////////////////////////

auto is_point_in_rect(float x, float y, SDL_FRect& rect)
{
    // TODO: would be good to make this consistent.
    // Problem comes from drawing SDL_RenderDrawRect that expects things in screen coordinates, but we do everything in eclidian coordinates.
    // y = to_screen_y(y);

    bool in_x = (x > rect.x) && (x < (rect.x + rect.w));
    bool in_y = (y > rect.y) && (y < (rect.y + rect.h));

    bool result{in_x && in_y};
    return result;
}

//////////////////////////////////////////////////////////////////////////////

template <typename Tp>
auto generate_points(Tp&& entity)
{
    linalg::Matrixf<4, 2> result;

    auto r = sdl_rect(entity);
    // float x = entity.X[0];
    // float y = entity.X[1];
    // float w = entity.dim[0];
    // float h = entity.dim[1];

    copy_from(result[0], {r.x + 0.f, r.y}); // tl
    copy_from(result[1], {r.x + r.w, r.y}); // tr
    copy_from(result[2], {r.x + r.w, r.y + r.h}); // br
    copy_from(result[3], {r.x + 0.f, r.y + r.h}); // bl

    return result;
}

//////////////////////////////////////////////////////////////////////////////

template <typename Tp>
auto minkowski_boundery(Tp&& entity, linalg::Vectorf<2> const& origin)
{
    auto points = generate_points(entity);

    float x = origin[0];
    float y = origin[1];
    points[0][0] += x;
    points[2][1] -= y;
    points[3][0] += x;
    points[3][1] -= y;

    float w = points[1][0] - points[0][0];
    float h = points[2][1] - points[0][1];

    return SDL_FRect{
        points[0][0],
        points[0][1],
        w,
        h};
}

#endif // GAME_ENTITY_HPP