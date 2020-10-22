#pragma once
#include "entity.hpp"
#include "fmt/core.h"
#include "linalg/matrix.hpp"
#include "linalg/trans.hpp"

namespace entity {
struct Crosshair {
    entity::Entity e;

    // This is where the player is pointing, not the angle of the entity.
    linalg::Vectorf<2> R;
};

inline auto make_crosshair()
{

    entity::Entity e{0};
    e.w = 20;
    e.h = 20;

    entity::Crosshair crosshair;
    crosshair.e = e;
    crosshair.R = {{0, 0}};

    return crosshair;
}

inline void update(Crosshair& crosshair, entity::Entity const& parent, float input, float dt)
{
    // Currently not using any dynamics.
    // a = /*kv +*/ su;
    // a acceleration
    // v velocity
    // u input
    //
    // k friction
    // s input scalar

    // Update rotation.
    auto&              R = crosshair.R;
    auto&              u = input;
    linalg::Vectorf<2> y;
    {
        float s = 10;
        float k = -0.2f;
        u *= s;

        // linalg::Matrixf<2, 2> A{{{0, 1}, {0, k}}};
        linalg::Matrixf<2, 2> A{{{0, 0}, {0, 0}}};
        linalg::Vectorf<2>    B{{1, 0}};

        // Currently not using any dynamics.
        auto Rdot = (R + (dt * A * R)) + ((dt * B) * u);

        y = R;
        R = Rdot;
    }

    fmt::print("angular pos:{0}  angular vel:{1}\n", y[0], y[1]);

    // Update position
    {
        auto& X = crosshair.e.X;

        // Move the crosshair to the center of the player.
        auto center = rect_center(parent);
        copy_from(X[0], center);

        // Create a vector rotated by the player's aim.
        linalg::Vectorf<2> m{{80, 0}};
        m = lrotzf(y[0]) * m;

        // Finally translate the crosshair away from the player using the m vector.
        X[0][0] += m[0];
        X[0][1] += m[1];
    }
}

}