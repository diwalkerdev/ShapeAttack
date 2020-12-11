#pragma once

#include "algorithms/find.hpp"
#include "collision/collision.hpp"
#include "containers/backfill_vector.hpp"
#include "entity/crosshair.hpp"
#include "entity/entity.hpp"
#include "entity/entityallocator.hpp"

namespace entity {

///////////////////////////////////////////////////////////////////////////////

struct Bullet {
    entity::Entity* s;
    entity::Entity* r;
    float          angle;

};

inline void integrate(Bullet& bullet, float dt)
{
    auto x = cosf(bullet.angle) * 100;
    auto y = -sinf(bullet.angle) * 100;

    linalg::Matrixf<2, 2> u{{{x, y}, {0, 0}}};
    entity::set_input(bullet.s, u);

    entity::integrate(bullet.s, dt);
}


///////////////////////////////////////////////////////////////////////////////
const float BULLET_WIDTH  = 10;
const float BULLET_HEIGHT = 10;

inline void init_bullet(Bullet* bullet, Allocator& alloca, float angle)
{
    int index;
    reserve(alloca, index);

    bullet->s     = &alloca.data[index];
    bullet->r     = &alloca.interpolated[index];
    bullet->angle = angle;

    auto* e = bullet->s;
    e->A    = {{{0, 0}, {0, 0}}};
    e->B    = {{{1, 0}, {0, 1}}};
    e->w    = BULLET_WIDTH;
    e->h    = BULLET_HEIGHT;
}

inline Bullet make_bullet(Allocator& alloca, float angle)
{
    Bullet bullet;
    init_bullet(&bullet, alloca, angle);

    return bullet;
}

struct Player {
    entity::Entity* s;
    entity::Entity* r;

    entity::Crosshair crosshair;
    backfill_vector<Bullet, 10> bullets;

    SDL_Texture* texture;
    float        health;
    float        restitution;

    void restore()
    {
        const float amount = 0.2;
        health += amount;
    }

    void respawn(linalg::Vectorf<2> const& point)
    {
        entity::Entity* e = s;
        e->X[0][0]        = point[0];
        e->X[0][1]        = point[1];
        health    = 1;
    }

    void fire()
    {
        if (bullets.size() < bullets.max_size())
        {
            auto& bullet = bullets.increase();
            bullet.angle = crosshair.rotX[0];

            center_on_center(bullet.s, this->s);
            *bullet.r = *bullet.s;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////

inline auto make_player(entity::Allocator& alloca, SDL_FRect rect) -> Player
{
    constexpr float const mass  = 1.f;
    constexpr float const imass = 1.f / mass;
    constexpr float const k     = 0.f * imass;
    constexpr float const b     = -3.f * imass; // friction coefficient

    float const x0     = rect.x;
    float const y0     = rect.y;
    float const width  = rect.w;
    float const height = rect.h;

    int index;
    reserve(alloca, index);

    Player player;
    player.s = &alloca.data[index];
    player.r = &alloca.interpolated[index];

    auto* entity = player.s;
    entity->w    = width;
    entity->h    = height;

    entity->X[0][0] = x0;
    entity->X[0][1] = y0;
    entity->X[1][0] = 0;
    entity->X[1][1] = 0;

    entity->A = {{{0.f, 1.f}, {k, b}}};
    entity->B = linalg::Matrixf<2, 2>::I();
    entity->B *= 500.f;

    player.crosshair   = entity::make_crosshair(alloca);
    player.health      = 0.5f;
    player.restitution = 0.5f;

    // We need to initialise all the bullets, even though they are not in use.
    // Hence why we use std::for_each with back() rather than a normal foreach
    // loop as bullets is effectively empty.
    std::for_each(player.bullets.begin(), player.bullets.back(), [&alloca](Bullet& bullet) {
        init_bullet(&bullet, alloca, 0);
    });

    return player;
}
///////////////////////////////////////////////////////////////////////////////

inline auto rect_center(Player const& player)
{
    return entity::rect_center(player.s);
}

///////////////////////////////////////////////////////////////////////////////

inline void hit(entity::Player& player, entity::Bullet const& bullet)
{
    const float amount = 0.2;

    player.health -= amount;
}

///////////////////////////////////////////////////////////////////////////////

inline void update_bullets(entity::Player&               player,
                           std::vector<entity::Player>&  players,
                           std::vector<SDL_FRect> const& hard_entities,
                           SDL_Rect const&               screen_rect,
                           float                         dt)
{
    auto& bullets = player.bullets;

    // Update the bullet positions.
    //
    for (auto& bullet : player.bullets)
    {
        integrate(bullet, dt);
    }

    // Check if bullets have hit any of the other players.
    //
    for (auto& other_player : players)
    {
        if (std::addressof(player) == std::addressof(other_player))
        {
            continue;
        }

        // TODO: Remove creating lambda on each iteration.
        auto collided_hard = [&other_player](Bullet const& bullet) {
            auto center   = rect_center(bullet.s);
            auto collided = collision::is_point_in_rect(center,
                                                        sdl_rect(other_player.s));
            if (collided)
            {
                hit(other_player, bullet);
            }
            return collided;
        };

        auto indices = algorithm::rfind_indices(bullets, collided_hard);

        for (auto i : indices)
        {
            fmt::print("FREE BULLET HARD {0}\n", i);
            bullets.remove(i);
        }
    }

    // Check if the bullets have hit any walls.
    //
    for (auto const& hard_entity : hard_entities)
    {
        // TODO: Remove creating lambda on each iteration.
        auto collided_hard = [&hard_entity](Bullet const& bullet) {
            auto& pX = bullet.s->X;

            linalg::Vectorf<2> origin{{pX[0][0], pX[0][1]}};
            return collision::is_point_in_rect(origin, hard_entity);
        };

        auto indices = algorithm::rfind_indices(bullets, collided_hard);

        for (auto i : indices)
        {
            fmt::print("FREE BULLET HARD {0}\n", i);
            bullets.remove(i);
        }
    }

    // Check if the bullets have left the screen.
    //
    auto within_screen = [&screen_rect](Bullet const& bullet) {
        auto center = rect_center(bullet.s);
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

namespace std {
inline void swap(entity::Bullet& self, entity::Bullet& other)
{
    // Deep swap the entity.
    auto const tmp = *self.s;
    *self.s        = *other.s;
    *other.s       = tmp;

    std::swap(self.angle, other.angle);
}
}
