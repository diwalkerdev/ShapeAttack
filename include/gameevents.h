#pragma once

#include "easing/core.hpp"
#include "linalg/matrix.hpp"
#include "msgpack.hpp"

// TODO Refactor. GameEvents is really a combination of event information and dev options.
// TODO Refactor. Do we still need lrup or can these be moved to the event handler?
struct GameEvents {
    GameEvents(easing::Easer& easer)
        : fire(easing::make_debounce_switch(easer, 10))
    {
    }
    easing::Debounce fire;

    int quit = 0;
    int hud  = 0;
    int l = 0, r = 0, u = 0, d = 0;

    linalg::Matrixf<2, 2> player_movement = 0;
    float player_rotation = 0;


    bool draw_vectors   = 0;
    bool draw_minkowski = 0;

    MSGPACK_DEFINE(draw_vectors, draw_minkowski);
};

