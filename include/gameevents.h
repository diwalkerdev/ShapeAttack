#ifndef GAMEEVENTS_H
#define GAMEEVENTS_H

#include "linalg/matrix.hpp"

struct GameEvents {
    int quit           = 0;
    int hud            = 0;
    int draw_vectors   = 0;
    int draw_minkowski = 0;
    int l = 0, r = 0, u = 0, d = 0;

    linalg::Matrixf<2, 2> player_movement = 0;
};

#endif