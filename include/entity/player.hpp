#pragma once

#include "containers/backfill_vector.hpp"
#include "entity/crosshair.hpp"
#include "entity/entity.hpp"

namespace entity {

///////////////////////////////////////////////////////////////////////////////

struct Bullet {
    entity::Entity e;
    float          angle;
};

inline void update_bullet(Bullet& bullet, float dt)
{
    auto x = cosf(bullet.angle) * 100;
    auto y = -sinf(bullet.angle) * 100;

    linalg::Matrixf<2, 2> u{{{x, y}, {0, 0}}};
    bullet.e.update(dt, u);
}

template <std::size_t Nm>
void update_bullets(backfill_vector<Bullet, Nm>& bullets, float dt)
{
    for (auto& bullet : bullets)
    {
        update_bullet(bullet, dt);
    }

    // TODO: Reuse bullets when they go off the screen.
}

///////////////////////////////////////////////////////////////////////////////

struct Player {
    entity::Entity    e;
    entity::Crosshair crosshair;
    backfill_vector<Bullet, 10> bullets;

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

    void fire()
    {
        if (bullets.size() < bullets.max_size())
        {
            Bullet bullet{{0},
                          crosshair.R[0]};
            bullet.e.A = {{{0, 0}, {0, 0}}};
            bullet.e.B = {{{1, 0}, {0, 1}}};
            bullet.e.X = e.X;
            center_on_center(bullet.e, e);
            bullet.e.w = 10;
            bullet.e.h = 10;
            bullets.push_back(bullet);
        }
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
