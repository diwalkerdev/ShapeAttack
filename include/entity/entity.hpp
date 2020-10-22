#pragma once

#include "linalg/matrix.hpp"
#include "recthelper.hpp"
#include "screen.h"
#include <SDL2/SDL.h>

namespace entity {

//////////////////////////////////////////////////////////////////////////////

struct Entity {
    float w, h;

    // State space representation.
    linalg::Matrixf<2, 2> X; // position, velocity.
    linalg::Matrixf<2, 2> Xdot; // velocity, acceleration.
    linalg::Matrixf<2, 2> Y;

    linalg::Matrixf<2, 2> A;
    linalg::Matrixf<2, 2> B;

    void update(float dt, linalg::Matrixf<2, 2> const& u)
    {
        Xdot = (X + (dt * A * X)) + (dt * B * u);
        Y    = X;
        X    = Xdot;
    }
};

//////////////////////////////////////////////////////////////////////////////

enum class EntityKinds {
    Boundary,
    Food
};

struct EntityStatic {
    SDL_Texture* texture;
    SDL_FRect    r;
    float        restitution;
    EntityKinds  kind_of;
    bool         alive;
};

//////////////////////////////////////////////////////////////////////////////

inline SDL_FRect sdl_rect(Entity const& entity)
{
    return {entity.X[0][0],
            entity.X[0][1],
            entity.w,
            entity.h};
}

inline auto rect_center(Entity const& entity)
{
    auto r = sdl_rect(entity);
    return rect_center(r);
}

inline auto sdl_rect(EntityStatic const& entity)
{
    return entity.r;
}

inline auto rect_center(EntityStatic const& entity)
{
    return ::rect_center(entity.r);
}

//////////////////////////////////////////////////////////////////////////////

template <typename Tp>
auto generate_points(Tp&& entity)
{
    linalg::Matrixf<4, 2> result;

    auto r = sdl_rect(entity);

    copy_from(result[0], {r.x + 0.f, r.y}); // tl
    copy_from(result[1], {r.x + r.w, r.y}); // tr
    copy_from(result[2], {r.x + r.w, r.y + r.h}); // br
    copy_from(result[3], {r.x + 0.f, r.y + r.h}); // bl

    return result;
}

//////////////////////////////////////////////////////////////////////////////

}
