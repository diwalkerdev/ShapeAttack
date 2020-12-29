#pragma once

#include "linalg/matrix.hpp"
#include "msgpack.hpp"

struct DevOptions {
    bool display_hud{};
    bool draw_vectors{};
    bool draw_minkowski{};

    MSGPACK_DEFINE(draw_vectors, draw_minkowski);
};
