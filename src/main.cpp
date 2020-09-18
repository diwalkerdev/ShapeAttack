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

#include <algorithm>
#include <complex>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>


static float turn = 0;


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

auto make_food()
{
    EntityStatic food;
    food.r           = {200.f, 100.f, 40.f, 40.f};
    food.restitution = 1.f;
    return food;
}

auto make_player()
{
    constexpr float mass  = 1.f;
    constexpr float imass = 1.f / mass;
    constexpr float k     = 0.f * imass;
    constexpr float b     = -3.f * imass; // friction coefficient

    Entity player{0};
    player.w           = 80.f;
    player.h           = 80.f;
    player.imass       = imass;
    player.k           = k;
    player.restitution = 0.5;

    player.X[0][0] = 100;
    player.X[0][1] = 100;
    player.X[1][0] = 0;
    player.X[1][1] = 0;

    player.A = {{{0.f, 1.f}, {k, b}}};
    player.B = linalg::Matrixf<2, 2>::I();
    player.B *= 500.f;
    return player;
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


int main()
{
    constexpr float dt = 1.0 / 30;

    SDL_Renderer* renderer;
    SDL_Event     e;
    kiss_array    objects;
    GameEvents    game_events;
    GameLoopTimer game_loop{0};
    int           draw;
    Entity        player = make_player();
    EntityStatic  food   = make_food();
    // food.texture = ...

    kiss_array_new(&objects);
    renderer = kiss_init("Hello kiss_sdl",
                         &objects,
                         SCREEN_WIDTH,
                         SCREEN_HEIGHT);

    if (renderer == nullptr)
    {
        printf("Renderer could not be created! SDL_Error: %s\n",
               SDL_GetError());
        return -1;
    }

    player.texture = load_texture(renderer,
                                  "/home/dwalker0044/Projects/ShapeAttack/res/player.png");

    DevHud dev_hud;
    auto*  dev_hud_texture = SDL_CreateTexture(renderer,
                                              SDL_PIXELFORMAT_ABGR8888,
                                              SDL_TEXTUREACCESS_TARGET,
                                              kiss_screen_width,
                                              kiss_screen_height);
    SDL_SetTextureBlendMode(dev_hud_texture, SDL_BLENDMODE_BLEND);

    GameHud game_hud;
    auto*   game_hud_texture = SDL_CreateTexture(renderer,
                                               SDL_PIXELFORMAT_ABGR8888,
                                               SDL_TEXTUREACCESS_TARGET,
                                               kiss_screen_width,
                                               kiss_screen_height);
    SDL_SetTextureBlendMode(game_hud_texture, SDL_BLENDMODE_BLEND);

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
        }

        // Update gameplay.
        auto player_copy = player;
        auto origin      = linalg::Vectorf<2>{{-80, -80}};
        auto boundary    = minkowski_boundery(food, origin);

        constexpr float dt_step = dt / 4;

        // TODO: different strategies? if collision first attempt then go smaller than dt_step?
        // Could also try a binary search like thing.

        linalg::Vectorf<2> c, r, p;

        bool collided;

        for (int i = 0; i < 4; ++i)
        {
            player.update(dt_step, game_events.player_movement);

            collided = is_point_in_rect(player.X[0][0],
                                        player.X[0][1],
                                        boundary);


            if (collided)
            {
                printf("Collided at iteration %d\n", i);

                // Go back to a position where the player is hasn't collided with the object.
                player = player_copy;

                linalg::Matrixf<2, 1> ux{{1.f, 0.f}};
                linalg::Matrixf<2, 1> uy{{0.f, 1.f}};

                auto  uxdot = T(ux) * ux;
                auto  uydot = T(uy) * uy;
                float ex    = uxdot[0] * (food.r.w * 0.5);
                float ey    = uydot[0] * (food.r.h * 0.5);

                c = sdl_rect_center(player);
                r = sdl_rect_center(food.r);

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

                auto c_norm = c - p;

                // c_norm is a vector between the two centroids. If we use for reflections then collisions
                // at the edge of the object to reflect outwards at the angle of c_norm and not perpendicular
                // which is what we want for collisions between two rectangles.
                bool x_edge;
                {
                    float x1 = std::abs(p[0] - player.X[0][0]);
                    float x2 = std::abs(p[0] - (player.X[0][0] + player.w));

                    float y1 = std::abs(p[1] - player.X[0][1]);
                    float y2 = std::abs(p[1] - (player.X[0][1] + player.h));

                    float xmin = std::min(x1, x2);
                    float ymin = std::min(y1, y2);

                    x_edge = (xmin < ymin);
                }

                // Calculate the time remaining after the collision.
                float dt_eval = dt - (dt_step * i);

                // Apply the stop vector.
                {
                    // linalg::Matrixf<2, 2> Binv{{{-0.0125, 0}, {0, -0.0125}}};
                    // linalg::Matrixf<2, 2> Xr{0.};
                    // copy_from(Xr[0], player.X[0]);

                    // auto Xtmp = player.X + (dt_eval * player.A * player.X);
                    // auto Rtmp = Binv * (1.f / dt_eval);
                    // auto u    = (Xtmp - Xr) * Rtmp;
                }


                {
                    float c_norm_x;
                    float c_norm_y;

                    float v_x;
                    float v_y;

                    // TODO write norm function.
                    float norm   = std::sqrt((c_norm[0] * c_norm[0]) + (c_norm[1] * c_norm[1]));
                    auto  v      = linalg::Vectorf<2>(player.X[1]);
                    auto  v_norm = std::sqrt((v[0] * v[0]) + (v[1] * v[1]));

                    float pv_x;
                    float pv_y;

                    // TODO: must be better way to refactor this.
                    if (x_edge)
                    {
                        c_norm_x = ((T(ux) * c_norm) * (1.f / norm))[0];
                        c_norm_y = 0.f;

                        // So like above we should get the x and y part of the
                        // velocity vector by taking the doc product with with
                        // the basis vectors,
                        v_x = c_norm_x * (T(ux) * (v * (1.f / v_norm)))[0];
                        v_y = c_norm_y * (T(uy) * (v * (1.f / v_norm)))[0];

                        // TODO: should compare restitutions and use the smallest one.
                        pv_x = v_x * player.restitution;
                        pv_y = 1.f;
                    }
                    else
                    {
                        c_norm_x = 0.f;
                        c_norm_y = ((T(uy) * c_norm) * (1.f / norm))[0];

                        v_x = c_norm_x * (T(ux) * (v * (1.f / v_norm)))[0];
                        v_y = c_norm_y * (T(uy) * (v * (1.f / v_norm)))[0];

                        pv_x = 1.f;
                        pv_y = v_y * player.restitution;
                    }

                    player.X[1][0] *= pv_x;
                    player.X[1][1] *= pv_y;
                    player.update(dt_eval, game_events.player_movement);
                }

                break;
            }
            else
            {
                player_copy = player;
            }
        }

        // Render gameplay.
        {
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);

            SDL_FRect dst = to_screen_rect(sdl_rect(player));
            SDL_Rect  src{0, 0, (int)player.w, (int)player.h};

            SDL_RenderCopyF(renderer,
                            player.texture,
                            &src,
                            &dst);

            SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
            SDL_FRect fdst = to_screen_rect(food.r);
            SDL_RenderFillRectF(renderer, &fdst);

            if (game_events.draw_minkowski)
            {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);
                auto dst = to_screen_rect(boundary);
                SDL_RenderDrawRectF(renderer, &dst);
            }

            SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
            if (game_events.draw_vectors)
            {
                draw_velocity_vector(renderer,
                                     player.X[0][0],
                                     player.X[0][1],
                                     player.Y[1][0],
                                     player.Y[1][1]);

                if (collided)
                {
                    draw_velocity_vector(renderer,
                                         r[0],
                                         r[1],
                                         p[0],
                                         p[1]);
                    std::cout << r;
                    std::cout << p;
                }
            }
        }

        {
            game_hud.update(game_loop.time_taken);
            game_hud.render(renderer, game_hud_texture);
        }

        // Render hud. This is a bit wasteful if the hud is not enabled
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

            SDL_RenderPresent(renderer);
        }

        game_loop.end();
        game_loop.delay();
    }

    kiss_clean(&objects);
}