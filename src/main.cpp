#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "devhud.h"
#include "entity.hpp"
#include "fmt/core.h"
#include "gameevents.h"
#include "gamehud.h"
#include "kiss_sdl.h"
#include "linalg/matrix.hpp"
#include "linalg/misc.hpp"
#include "linalg/trans.hpp"
#include "misc.hpp"
#include "recthelper.hpp"
#include "screen.h"
#include "shapes.hpp"
#include "typedefs.h"
// #include "spdlog/spdlog.h"
#include "msgpack.hpp"

#include <algorithm>
#include <complex>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

void handle_input(SDL_Event& event, GameEvents& game_events)
{
    if (event.type == SDL_KEYUP)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
            game_events.u = 0;
            break;
        case SDLK_DOWN:
            game_events.d = 0;
            break;
        case SDLK_LEFT:
            game_events.l = 0;
            break;
        case SDLK_RIGHT:
            game_events.r = 0;
            break;
        case SDLK_LSHIFT:
            break;
        case SDLK_SPACE:
            break;
        default:
            break;
        }
    }
    else if (event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
            game_events.u = 1;
            break;
        case SDLK_DOWN:
            game_events.d = 1;
            break;
        case SDLK_LEFT:
            game_events.l = 1;
            break;
        case SDLK_RIGHT:
            game_events.r = 1;
            break;
        case SDLK_LSHIFT:
            break;
        case SDLK_SPACE:
            break;
        case SDLK_h:
            game_events.hud = game_events.hud ? 0 : 1;
            printf("hud\n");
            break;
        case SDLK_ESCAPE:
            game_events.quit = 1;
            break;
        default:
            break;
        }
    }

    {
        float x_axis = game_events.r - game_events.l;
        float y_axis = game_events.u - game_events.d;

        if (x_axis != 0 && y_axis != 0)
        {
            float norm = std::sqrt((x_axis * x_axis) + (y_axis * y_axis));
            x_axis /= norm;
            y_axis /= norm;
        }

        game_events.player_movement = {
            {{0.f, 0.f},
             {x_axis, y_axis}}};
    }
}

//////////////////////////////////////////////////////////////////////////////

template <typename Tp, std::size_t M, std::size_t N>
void draw(SDL_Renderer* renderer, linalg::Matrix<Tp, M, N>& mat)
{
    for (auto i : irange<M - 1>())
    {
        SDL_RenderDrawLineF(renderer,
                            mat[i][0],
                            to_screen_y(mat[i][1]),
                            mat[i + 1][0],
                            to_screen_y(mat[i + 1][1]));
    }
}

linalg::Matrixf<4, 3> vector_head{{{+0, +0, +1},
                                   {-1, -1, +1},
                                   {-1, +1, +1},
                                   {+0, +0, +1}}};


void draw_velocity_vector(SDL_Renderer* renderer, float px, float py, float vx, float vy)
{
    auto arrow = vector_head;
    // TODO use column operator.
    for (auto row : iter(arrow))
    {
        row[0] *= 10;
        row[1] *= 10;
    }

    float theta = std::atan2(-vy, vx);

    // TODO *=
    arrow = arrow * rtransf(theta, px + vx, py + vy);

    linalg::Matrixf<2, 3> vector_tail;
    vector_tail[0][0] = arrow[0][0];
    vector_tail[0][1] = arrow[0][1];
    vector_tail[0][2] = arrow[0][2];

    vector_tail[1][0] = px;
    vector_tail[1][1] = py;
    vector_tail[1][2] = 1;

    draw(renderer, arrow);
    draw(renderer, vector_tail);
}

//////////////////////////////////////////////////////////////////////////////

struct GameLoopTimer {
    int32 start_frame, end_frame, time_taken, delay_time;
    float fps;

    void start()
    {
        start_frame = SDL_GetTicks();
    }

    void end()
    {
        end_frame  = SDL_GetTicks();
        time_taken = end_frame - start_frame;

        if (time_taken < 33)
        {
            delay_time = 33 - time_taken;
        }
        else
        {
            printf("WARNING FRAME TOOK LONGER THAN 33ms");
            delay_time = 0;
        }

        fps = 1000.0 / float(time_taken + delay_time);
    }

    void delay()
    {
        SDL_Delay(delay_time);
    }
};

SDL_Texture* load_texture(SDL_Renderer* renderer,
                          std::string   path)
{
    //The final texture
    SDL_Texture* new_texture = NULL;

    //Load image at specified path
    SDL_Surface* loaded_surface = IMG_Load(path.c_str());
    if (loaded_surface == NULL)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n",
               path.c_str(),
               IMG_GetError());
        exit(-1);
    }

    //Create texture from surface pixels
    new_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    if (new_texture == NULL)
    {
        printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
    }

    printf("Loaded %s\n", path.c_str());

    //Get rid of old loaded surface
    SDL_FreeSurface(loaded_surface);

    return new_texture;
}

void println(char const* message, float number)
{
    printf("%s", message);
    printf(": %f\n", number);
}

void create_game_objects(linalg::Vectorf<2>         origin,
                         std::vector<EntityStatic>& game_entities,
                         std::vector<EntityStatic>& walls,
                         std::vector<SDL_FRect>&    soft_boundaries,
                         std::vector<SDL_FRect>&    hard_boundaries)
{
    game_entities.push_back(make_food());
    walls.push_back(make_wall());

    for (EntityStatic& entity : walls)
    {
        hard_boundaries.push_back(minkowski_boundary(entity, origin));
    }
    for (EntityStatic& entity : game_entities)
    {
        soft_boundaries.push_back(minkowski_boundary(entity, origin));
    }
}


void detect_hard_collisions(float                            dt,
                            float                            dt_step,
                            int                              loop_idx,
                            GameEvents const&                game_events,
                            Player&                          player,
                            std::vector<EntityStatic> const& walls,
                            std::vector<SDL_FRect> const&    hard_boundaries,
                            bool&                            collided)
{
    Player player_copy = player;
    for (int wall_idx = 0;
         (wall_idx < walls.size()) && !collided;
         ++wall_idx)
    {
        auto& boundary = hard_boundaries[wall_idx];
        auto& entity   = walls[wall_idx];

        auto& X = player.e.X;

        {
            linalg::Vectorf<2> c, r, p;

            collided = is_point_in_rect(player.e.X[0][0],
                                        player.e.X[0][1],
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

                linalg::Matrixf<2, 1> ux{{1.f, 0.f}};
                linalg::Matrixf<2, 1> uy{{0.f, 1.f}};

                // TODO: LA, automatically return float from the dot product below (eliminate [0]).
                auto  uxdot = (T(ux) * ux)[0];
                auto  uydot = (T(uy) * uy)[0];
                float ex    = uxdot * (entity.r.w * 0.5);
                float ey    = uydot * (entity.r.h * 0.5);

                c = sdl_rect_center(player.e);
                r = sdl_rect_center(entity.r);

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

                    float x1 = std::abs(p[0] - player.e.X[0][0]);
                    float x2 = std::abs(p[0] - (player.e.X[0][0] + player.e.w));

                    float y1 = std::abs(p[1] - player.e.X[0][1]);
                    float y2 = std::abs(p[1] - (player.e.X[0][1] + player.e.h));

                    float xmin = std::min(x1, x2);
                    float ymin = std::min(y1, y2);

                    x_edge = (xmin < ymin);
                }

                {
                    // TODO write norm function.
                    auto c_norm = norm(c_vec);
                    auto v      = linalg::Vectorf<2>(player.e.X[1]);
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
                        pv_x = v_x * player.e.restitution;
                        pv_y = uydot; // allows gliding.
                    }
                    else
                    {
                        auto c_norm_x = 0.f;
                        auto c_norm_y = ((T(uy) * c_norm))[0];

                        auto v_y = (c_norm_y * (T(uy) * v_norm))[0];

                        pv_x = uxdot; // allows gliding.
                        pv_y = v_y * player.e.restitution;
                    }

                    player.e.X[1][0] *= pv_x;
                    player.e.X[1][1] *= pv_y;

                    // Calculate the time remaining after the collision.
                    float dt_eval = dt - (dt_step * loop_idx);
                    player.e.update(dt_eval, game_events.player_movement);
                }
            } // if collision.
        } // collision block.
    } // game entity loop.
}

auto detect_soft_collisions(Player&                    player,
                            std::vector<EntityStatic>& game_entities,
                            std::vector<SDL_FRect>&    soft_boundaries)
{
    std::vector<int> interacted_with;

    for (int entity_idx = 0;
         (entity_idx < game_entities.size());
         ++entity_idx)
    {
        auto& boundary = soft_boundaries[entity_idx];
        auto& entity   = game_entities[entity_idx];

        bool collided = is_point_in_rect(player.e.X[0][0],
                                         player.e.X[0][1],
                                         boundary);

        if (collided && entity.alive)
        {
            switch (entity.kind_of)
            {
            case EntityKinds::Food: {
                printf("Collided with food\n");
                entity.alive = false;
                player.eat(0.5);
                break;
            }
            }
        }
    }
}

#include "propertyeditor.hpp"

int main()
{
    constexpr float dt      = 1.f / 30.f;
    constexpr float dt_step = dt / 4.f;

    SDL_Renderer*      renderer;
    SDL_Event          e;
    kiss_array         objects;
    GameEvents         game_events;
    GameLoopTimer      game_loop{0};
    int                draw;
    auto               player = make_player();
    linalg::Vectorf<2> origin{{-80, -80}};

    std::vector<EntityStatic> game_entities;
    std::vector<EntityStatic> walls;
    std::vector<SDL_FRect>    soft_boundaries;
    std::vector<SDL_FRect>    hard_boundaries;

    kiss_array_new(&objects);
    renderer = kiss_init("Hello kiss_sdl",
                         &objects,
                         SCREEN_WIDTH,
                         SCREEN_HEIGHT);

    // Note, you MUST create the hud after kiss_init.
    SDL_Rect kiss_screen{0,
                         0,
                         kiss_screen_width,
                         kiss_screen_height};
    DevHud   dev_hud(kiss_screen,
                   kiss_screen_width / 2,
                   kiss_screen_height / 2);
    GameHud  game_hud;

    kiss_window editor_window;
    WindowData  window_data;
    make_window_from_values(editor_window,
                            kiss_screen,
                            kiss_screen_width / 2,
                            kiss_screen_height / 2,
                            window_data);

    if (renderer == nullptr)
    {
        printf("Renderer could not be created! SDL_Error: %s\n",
               SDL_GetError());
        return -1;
    }

    player.texture = load_texture(renderer,
                                  "/home/dwalker0044/Projects/Untitled2D/res/player.png");

    auto* dev_hud_texture = SDL_CreateTexture(renderer,
                                              SDL_PIXELFORMAT_ABGR8888,
                                              SDL_TEXTUREACCESS_TARGET,
                                              kiss_screen_width,
                                              kiss_screen_height);
    SDL_SetTextureBlendMode(dev_hud_texture, SDL_BLENDMODE_BLEND);


    auto* game_hud_texture = SDL_CreateTexture(renderer,
                                               SDL_PIXELFORMAT_ABGR8888,
                                               SDL_TEXTUREACCESS_TARGET,
                                               kiss_screen_width,
                                               kiss_screen_height);
    SDL_SetTextureBlendMode(game_hud_texture, SDL_BLENDMODE_BLEND);

    create_game_objects(origin,
                        game_entities,
                        walls,
                        soft_boundaries,
                        hard_boundaries);

    assert(game_entities.size() == soft_boundaries.size());
    assert(walls.size() == hard_boundaries.size());

    while (!game_events.quit)
    {
        game_loop.start();

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                game_events.quit = 1;
            }

            kiss_window_event(&dev_hud.window, &e, &draw);
            handle_input(e, game_events);
            dev_hud.handle_events(&e, &draw, game_events);
            game_hud.handle_events(&e, &draw, game_events);
            window_handle_events(&e, &draw, window_data);
        }

        // Update gameplay.
        auto player_copy = player;
        bool collided    = false;

        // Collision detection loop.
        //
        // TODO: different strategies? if collision first attempt then go smaller than dt_step?
        // Could also try a binary search like thing.
        // TODO: this doesn't handle colliding with several objects at once.
        {
            for (int loop_idx = 0;
                 (loop_idx < 4) && !collided;
                 ++loop_idx)
            {
                player.e.update(dt_step, game_events.player_movement);

                detect_hard_collisions(dt, dt_step, loop_idx, game_events, player, walls, hard_boundaries, collided);
                detect_soft_collisions(player, game_entities, soft_boundaries);
            }

            player.update();

            // Player status, detect game over events.
            if (player.hunger < 0.f)
            {
                printf("Player starved.\n");
                player.hunger = 0.5;
                // TODO: Game over event.
            }
        }

        // Render player.
        {
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);

            SDL_FRect dst = to_screen_rect(sdl_rect(player.e));
            SDL_Rect  src{0, 0, (int)player.e.w, (int)player.e.h};

            SDL_RenderCopyF(renderer,
                            player.texture,
                            &src,
                            &dst);
        }

        // Render game entities.
        {
            SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
            for (auto& entity : game_entities)
            {
                if (entity.alive)
                {
                    SDL_FRect fdst = to_screen_rect(entity.r);
                    SDL_RenderFillRectF(renderer, &fdst);
                }
            }

            SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xff);
            for (auto& entity : walls)
            {
                SDL_FRect fdst = to_screen_rect(entity.r);
                SDL_RenderFillRectF(renderer, &fdst);
            }

            if (game_events.draw_minkowski)
            {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);

                for (auto& boundary : hard_boundaries)
                {
                    auto dst = to_screen_rect(boundary);
                    SDL_RenderDrawRectF(renderer, &dst);
                }

                for (auto& boundary : soft_boundaries)
                {
                    auto dst = to_screen_rect(boundary);
                    SDL_RenderDrawRectF(renderer, &dst);
                }
            }

            SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
            if (game_events.draw_vectors)
            {
                draw_velocity_vector(renderer,
                                     player.e.X[0][0],
                                     player.e.X[0][1],
                                     player.e.Y[1][0],
                                     player.e.Y[1][1]);
            }
        }

        // Render game hud.
        {
            game_hud.update(player);
            game_hud.render(renderer, game_hud_texture);
        }

        // Render dev hud. This is a bit wasteful if the hud is not enabled
        // however for not it is good to do so we can keep an eye on CPU usage.
        {
            dev_hud.update(game_loop.time_taken,
                           game_loop.fps,
                           game_events.player_movement);

            dev_hud.render(renderer, dev_hud_texture);
        }

        // Copy textures from kiss to the screen.
        {
            SDL_RenderCopy(renderer,
                           game_hud_texture,
                           &game_hud.window.rect,
                           &game_hud.window.rect);

            if (game_events.hud)
            {
                SDL_RenderCopy(renderer,
                               dev_hud_texture,
                               &dev_hud.window.rect,
                               &dev_hud.window.rect);
            }

            window_render(renderer, nullptr, window_data, editor_window);
            SDL_RenderPresent(renderer);
        }


        game_loop.end();
        game_loop.delay();
    }

    kiss_clean(&objects);
}