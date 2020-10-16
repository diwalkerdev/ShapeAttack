#ifndef GAMEEVENTS_H
#define GAMEEVENTS_H

#include "linalg/matrix.hpp"
#include "msgpack.hpp"

// TODO Refactor. GameEvents is really a combination of event information and dev options.
struct GameEvents {
    int quit = 0;
    int hud  = 0;
    int l = 0, r = 0, u = 0, d = 0;

    linalg::Matrixf<2, 2> player_movement = 0;

    bool draw_vectors   = 0;
    bool draw_minkowski = 0;

    MSGPACK_DEFINE(draw_vectors, draw_minkowski);
};

#endif
