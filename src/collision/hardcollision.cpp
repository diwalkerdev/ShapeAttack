#include "collision/core.hpp"
#include "entity/core.hpp"
#include "gameevents.h"
#include "recthelper.hpp"
#include <SDL2/SDL.h>
#include <vector>

namespace collision {

void detect_hard_collisions(float                                    dt,
                            float                                    dt_step,
                            int                                      loop_idx,
                            GameEvents const&                        game_events,
                            entity::Player&                          player,
                            std::vector<entity::EntityStatic> const& walls,
                            std::vector<SDL_FRect> const&            hard_boundaries,
                            bool&                                    collided)
{
    entity::Player player_copy = player;
    for (int wall_idx = 0;
         (wall_idx < walls.size()) && !collided;
         ++wall_idx)
    {
        auto& boundary = hard_boundaries[wall_idx];
        auto& entity   = walls[wall_idx];


        {
            linalg::Vectorf<2> c, r, p;

            collided = collision::is_point_in_rect(player.s->X[0][0],
                                                   player.s->X[0][1],
                                                   boundary);

            if (!collided)
            {
                player_copy = player;
            }
            else
            {
                // printf("Collided at iteration %d\n", i);

                // Go back to a position where the player is hasn't collided with the object.
                player = player_copy;
                auto& pX = player.s->X;
                auto& pw = player.s->w;
                auto& ph = player.s->h;

                linalg::Matrixf<2, 1> ux{{1.f, 0.f}};
                linalg::Matrixf<2, 1> uy{{0.f, 1.f}};

                // TODO: LA, automatically return float from the dot product below (eliminate [0]).
                auto  uxdot = (T(ux) * ux)[0];
                auto  uydot = (T(uy) * uy)[0];
                float ex    = uxdot * (entity.rect.w * 0.5);
                float ey    = uydot * (entity.rect.h * 0.5);

                c = entity::rect_center(player);
                r = entity::rect_center(entity);

                auto d = T(c - r);

                float dx = (d * ux)[0];
                float dy = (d * uy)[0];

                if (dx > ex)
                {
                    dx = ex;
                }
                if (dx < -ex)
                {
                    dx = -ex;
                }

                if (dy > ey)
                {
                    dy = ey;
                }
                if (dy < -ey)
                {
                    dy = -ey;
                }

                p = r + (dx * ux) + (dy * uy);

                auto c_vec = c - p;

                // NOTE: Alot of the code below is  more complicated than it actually needs to be
                // handling rectangles that don't rotate.

                // c_norm is a vector between the two centroids. If we use for reflections then collisions
                // at the edge of the object to reflect outwards at the angle of c_norm and not perpendicular
                // which is what we want for collisions between two rectangles.
                bool x_edge;
                {

                    float x1 = std::abs(p[0] - pX[0][0]);
                    float x2 = std::abs(p[0] - (pX[0][0] + pw));

                    float y1 = std::abs(p[1] - pX[0][1]);
                    float y2 = std::abs(p[1] - (pX[0][1] + ph));

                    float xmin = std::min(x1, x2);
                    float ymin = std::min(y1, y2);

                    x_edge = (xmin < ymin);
                }

                {
                    // TODO write norm function.
                    auto c_norm = norm(c_vec);
                    auto v      = linalg::Vectorf<2>(pX[1]);
                    auto v_norm = norm(v);

                    float pv_x;
                    float pv_y;

                    // TODO: must be better way to refactor this.
                    if (x_edge)
                    {
                        auto c_norm_x = ((T(ux) * c_norm))[0];
                        auto c_norm_y = 0.f;

                        // So like above we should get the x and y part of the
                        // velocity vector by taking the doc product with with
                        // the basis vectors,
                        auto v_x = (c_norm_x * (T(ux) * (v_norm)))[0];

                        // TODO: should compare restitutions and use the smallest one.
                        pv_x = v_x * player.restitution;
                        pv_y = uydot; // allows gliding.
                    }
                    else
                    {
                        auto c_norm_x = 0.f;
                        auto c_norm_y = ((T(uy) * c_norm))[0];

                        auto v_y = (c_norm_y * (T(uy) * v_norm))[0];

                        pv_x = uxdot; // allows gliding.
                        pv_y = v_y * player.restitution;
                    }

                    pX[1][0] *= pv_x;
                    pX[1][1] *= pv_y;

                    // Calculate the time remaining after the collision.
                    float dt_eval = dt - (dt_step * loop_idx);
                    entity::set_input(player.s, game_events.player_movement);
                    entity::integrate(player.s, dt_eval);
                }
            } // if collision.
        } // collision block.
    } // game entity loop.
}

}
