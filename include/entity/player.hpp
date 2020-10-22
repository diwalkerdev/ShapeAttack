#pragma once

#include "entity/crosshair.hpp"
#include "entity/entity.hpp"

namespace entity {

///////////////////////////////////////////////////////////////////////////////

struct Player {
    entity::Entity    e;
    entity::Crosshair crosshair;

    SDL_Texture* texture;
    float        hunger;
    float        restitution;

    void update()
    {
        hunger -= 0.001;
    }

    void eat(float amount)
    {
        hunger += amount;
        hunger = (hunger <= 1.f) ? hunger : 1.f;
    }
};

///////////////////////////////////////////////////////////////////////////////

inline auto make_player(float x0, float y0, float width, float height)
{
    constexpr float mass  = 1.f;
    constexpr float imass = 1.f / mass;
    constexpr float k     = 0.f * imass;
    constexpr float b     = -3.f * imass; // friction coefficient

    Entity entity{0};
    entity.w = width;
    entity.h = height;

    entity.X[0][0] = x0;
    entity.X[0][1] = y0;
    entity.X[1][0] = 0;
    entity.X[1][1] = 0;

    entity.A = {{{0.f, 1.f}, {k, b}}};
    entity.B = linalg::Matrixf<2, 2>::I();
    entity.B *= 500.f;

    Player player;
    player.e           = entity;
    player.crosshair   = entity::make_crosshair();
    player.hunger      = 0.5f;
    player.restitution = 0.5f;

    return player;
}

inline auto rect_center(Player const& player)
{
    return entity::rect_center(player.e);
}

}