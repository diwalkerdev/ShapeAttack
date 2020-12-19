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

    linalg::Matrixf<2, 2> u;
};

inline void set_input(Entity* e, linalg::Matrixf<2, 2> const& u)
{
    e->u = u;
}

inline void integrate(Entity* e, float dt)
{
    e->Xdot = (e->X + (dt * e->A * e->X)) + ((dt * e->B) * e->u);
    e->Y    = e->X;
    e->X    = e->Xdot;
}

// For state(pos, vel) with 2 dimensions, X is:
// px, py
// vx, vy
//
// However rotation has state(angle, angular vel) but is one dimensional, so X is:
// ox, o=theta
// wx  w=omega
//
// The input vector u needs to be 2x1 as you can potentially input a position
// or an angular velocity.
//
// Then we want the results A*X and B*u to yield 2x1 matrices.
// Therefore A and B need to be 2x2 matrices.
// The easy way to think about this is if you imaging multiplying X and u by an
// identity matrix. You'd get back the original vectors since I does nothing.
struct EntityRotation {
    linalg::Matrixf<2, 1> X; // position, velocity.
    linalg::Matrixf<2, 1> Xdot; // velocity, acceleration.
    linalg::Matrixf<2, 1> Y;

    linalg::Matrixf<2, 2> A;
    linalg::Matrixf<2, 2> B;

    linalg::Matrixf<2, 1> u;
};

inline void integrate(EntityRotation* e, float dt)
{
    e->Xdot = (e->X + (dt * e->A * e->X)) + ((dt * e->B) * e->u);
    e->Y    = e->X;
    e->X    = e->Xdot;
}

inline void set_input(EntityRotation* e, float uo, float uw)
{
    e->u[0] = uo;
    e->u[1] = uw;
}

inline void set_input(EntityRotation* e, linalg::Vectorf<2> const& u)
{
    e->u = u;
}


//////////////////////////////////////////////////////////////////////////////

enum class EntityKinds {
    Boundary,
    Food
};

struct EntityStatic {
    SDL_Texture* texture;
    SDL_FRect    rect;
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

inline SDL_FRect sdl_rect(Entity const* entity)
{
    return {entity->X[0][0],
            entity->X[0][1],
            entity->w,
            entity->h};
}

inline auto rect_center(Entity const& entity)
{
    auto r = sdl_rect(entity);
    return rect_center(r);
}

inline auto rect_center(Entity const* entity)
{
    auto r = sdl_rect(entity);
    return rect_center(r);
}

inline auto sdl_rect(EntityStatic const& entity)
{
    return entity.rect;
}

inline auto rect_center(EntityStatic const& entity)
{
    return ::rect_center(entity.rect);
}

inline auto center_on_point(Entity& a, linalg::Vectorf<2> const& center)
{
    copy_from(a.X[0], center);
    a.X[0] += linalg::Vectorf<2>{{-a.w / 2.f, -a.h / 2.f}};
}

inline auto center_on_point(Entity* a, linalg::Vectorf<2> const& center)
{
    copy_from(a->X[0], center);
    a->X[0] += linalg::Vectorf<2>{{-a->w / 2.f, -a->h / 2.f}};
}

inline auto center_on_center(Entity& a, Entity const& b)
{
    center_on_point(a, rect_center(b));
}

inline auto center_on_center(Entity& a, Entity const* b)
{
    center_on_point(a, rect_center(b));
}

inline auto center_on_center(Entity* a, Entity const* b)
{
    center_on_point(a, rect_center(b));
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
