#pragma once

#include "collision/collision.hpp"
#include "collision/minkowski.hpp"
#include "entity/core.hpp"
#include "gameevents.h"

namespace collision {

void detect_hard_collisions(float                                    dt,
                            float                                    dt_step,
                            int                                      loop_idx,
                            GameEvents const&                        game_events,
                            entity::Player&                          player,
                            std::vector<entity::EntityStatic> const& walls,
                            std::vector<SDL_FRect> const&            hard_boundaries,
                            bool&                                    collided);

void detect_soft_collisions(entity::Player&                    player,
                            std::vector<entity::EntityStatic>& game_entities,
                            std::vector<SDL_FRect>&            soft_boundaries);

}