#pragma once

#include "easing/core.hpp"
#include "linalg/matrix.hpp"
#include "msgpack.hpp"

// TODO Split events and dev options into seperate files.
struct GameEvents {
    GameEvents(easing::Easer& easer)
        : fire(easing::make_debounce_switch(easer, 200))
    {
    }

    int quit = 0;

    linalg::Matrixf<2, 2> player_movement = 0;
    float                 player_rotation = 0;
    easing::Debounce      fire;
};

struct DevOptions {
    bool display_hud{};
    bool draw_vectors{};
    bool draw_minkowski{};

    MSGPACK_DEFINE(draw_vectors, draw_minkowski);
};
