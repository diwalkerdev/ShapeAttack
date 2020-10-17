#pragma once

#include "entity/entity.hpp"

namespace entity {

struct Player {
    entity::Entity e;

    SDL_Texture* texture;
    float        hunger;

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

inline auto make_player()
{
    constexpr float mass  = 1.f;
    constexpr float imass = 1.f / mass;
    constexpr float k     = 0.f * imass;
    constexpr float b     = -3.f * imass; // friction coefficient

    Entity entity{0};
    entity.w           = 80.f;
    entity.h           = 80.f;
    entity.imass       = imass;
    entity.k           = k;
    entity.restitution = 0.5;

    entity.X[0][0] = 100;
    entity.X[0][1] = 100;
    entity.X[1][0] = 0;
    entity.X[1][1] = 0;

    entity.A = {{{0.f, 1.f}, {k, b}}};
    entity.B = linalg::Matrixf<2, 2>::I();
    entity.B *= 500.f;

    Player player;
    player.e      = entity;
    player.hunger = 0.5f;

    return player;
}

}