#pragma once

#include "algorithms/find.hpp"
#include "collision/collision.hpp"
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


///////////////////////////////////////////////////////////////////////////////

inline Bullet make_bullet(float angle)
{
    Bullet bullet{{0}, angle};
    bullet.e.A = {{{0, 0}, {0, 0}}};
    bullet.e.B = {{{1, 0}, {0, 1}}};
    // bullet.e.X = e.X;
    // center_on_center(bullet.e, e);
    bullet.e.w = 10;
    bullet.e.h = 10;

    return bullet;
}

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
        fmt::print("FIRE\n");
        if (bullets.size() < bullets.max_size())
        {
            auto bullet = make_bullet(crosshair.R[0]);
            // Bullet bullet{{0},
            //               crosshair.R[0]};
            // bullet.e.A = {{{0, 0}, {0, 0}}};
            // bullet.e.B = {{{1, 0}, {0, 1}}};
            // bullet.e.w = 10;
            // bullet.e.h = 10;

            bullet.e.X = e.X;
            center_on_center(bullet.e, e);
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
///////////////////////////////////////////////////////////////////////////////

inline auto rect_center(Player const& player)
{
    return entity::rect_center(player.e);
}
///////////////////////////////////////////////////////////////////////////////

template <typename Tp, std::size_t Nm>
void update_bullets(backfill_vector<Tp, Nm>&           bullets,
                    std::vector<entity::Player> const& players,
                    std::vector<SDL_FRect> const&      hard_entities,
                    SDL_Rect const&                    screen_rect,
                    float                              dt)
{
    for (auto& bullet : bullets)
    {
        update_bullet(bullet, dt);
    }

    for (auto const& player : players)
    {
        // TODO: Remove creating lambda on each iteration.
        auto collided_hard = [&player](Tp const& bullet) {
            auto center = rect_center(bullet.e);
            return collision::is_point_in_rect(center, sdl_rect(player.e));
        };

        auto indices = algorithm::rfind_indices(bullets, collided_hard);

        for (auto i : indices)
        {
            fmt::print("FREE BULLET HARD {0}\n", i);
            bullets.remove(i);
        }
    }
    for (auto const& hard_entity : hard_entities)
    {
        // TODO: Remove creating lambda on each iteration.
        auto collided_hard = [&hard_entity](Tp const& bullet) {
            linalg::Vectorf<2> origin{{bullet.e.X[0][0], bullet.e.X[0][1]}};
            return collision::is_point_in_rect(origin, hard_entity);
        };

        auto indices = algorithm::rfind_indices(bullets, collided_hard);

        for (auto i : indices)
        {
            fmt::print("FREE BULLET HARD {0}\n", i);
            bullets.remove(i);
        }
    }


    auto within_screen = [&screen_rect](Tp const& bullet) {
        auto center = rect_center(bullet.e);
        return !collision::is_point_in_rect(center, screen_rect);
    };

    // TODO: Is there a way to do this which is less error prone?
    // Removing several elements must be done in reverse index order.
    auto indices = algorithm::rfind_indices(bullets, within_screen);

    for (auto i : indices)
    {
        fmt::print("FREE BULLET SCREEN {0}\n", i);
        bullets.remove(i);
    }
}
}
