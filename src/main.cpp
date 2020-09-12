#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "devhud.h"
#include "gameevents.h"
#include "fmt/core.h"
#include "kiss_sdl.h"
#include "linalg/matrix.hpp"
#include "linalg/misc.hpp"
#include "linalg/trans.hpp"
#include "misc.hpp"
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

static constexpr int SCREEN_WIDTH       = 640;
static constexpr int SCREEN_HEIGHT      = 400;
static constexpr int HALF_SCREEN_WIDTH  = SCREEN_WIDTH / 2;
static constexpr int HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2;

static float turn = 0;



//////////////////////////////////////////////////////////////////////////////
auto to_screen_y(float y) -> float
{
    return SCREEN_HEIGHT - y;
}

auto to_screen_y(int y) -> int
{
    return SCREEN_HEIGHT - y;
}

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

//////////////////////////////////////////////////////////////////////////////

struct Entity {
    SDL_Texture* texture;

    // State space representation.
    linalg::Matrixf<2, 2> X; // position, velocity.
    linalg::Matrixf<2, 2> Xdot; // velocity, acceleration.
    linalg::Matrixf<2, 2> Y;

    linalg::Matrixf<2, 2> A;
    linalg::Matrixf<2, 2> B;

    float imass;
    float k; // friction.

    void update(float dt, linalg::Matrixf<2, 2> const& u)
    {
        Xdot = (X + (dt * A * X)) + ((dt * B) * u);
        Y    = X;
        X    = Xdot;
    }
};

auto make_player()
{
    Entity player{0};
    float  mass  = 1.f;
    player.imass = 1.f / mass;
    player.k     = -mass * 0.7f;

    player.X[0][0] = 100;
    player.X[0][1] = 100;
    player.X[1][0] = 0;
    player.X[1][1] = 0;

    player.A = {
        {{+0.f, +1.f},
         {+0.f, player.k}}};
    player.A *= player.imass;

    player.B = linalg::Matrixf<2, 2>::I();
    player.B *= 80.f;

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


void draw_velocity_vector(SDL_Renderer* renderer, Entity const& entity)
{
    auto new_arrow = vector_head;
    for (auto row : iter(new_arrow))
    {
        row[0] *= 10;
        row[1] *= 10;
    }

    float offset_x = entity.Y[0][0];
    float offset_y = entity.Y[0][1];

    float theta       = std::atan2(-entity.Y[1][1], entity.Y[1][0]);
    auto  trans_arrow = new_arrow * rtransf(theta, offset_x + entity.Y[1][0], offset_y + entity.Y[1][1]);

    linalg::Matrixf<2, 3> vector_tail;
    vector_tail[0][0] = trans_arrow[0][0];
    vector_tail[0][1] = trans_arrow[0][1];
    vector_tail[0][2] = trans_arrow[0][2];

    vector_tail[1][0] = offset_x;
    vector_tail[1][1] = offset_y;
    vector_tail[1][2] = 1;

    draw(renderer, trans_arrow);
    draw(renderer, vector_tail);
}

//////////////////////////////////////////////////////////////////////////////

void button_event(kiss_button* button, SDL_Event* e, int* draw, int* quit)
{
    if (kiss_button_event(button, e, draw))
    {
        printf("button event\n");
        *quit = 1;
    }
}


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
    float         dt = 1.0 / 30;
    SDL_Renderer* renderer;
    SDL_Event     e;
    kiss_array    objects;
    GameEvents    game_events;
    GameLoopTimer game_loop{0};
    int           draw;
    Entity        player = make_player();

    std::string window_title("Hello kiss_sdl");
    kiss_array_new(&objects);
    renderer = kiss_init(window_title.c_str(),
                         &objects,
                         SCREEN_WIDTH,
                         SCREEN_HEIGHT);

    if (renderer == nullptr)
    {
        printf("Renderer could not be created! SDL_Error: %s\n",
               SDL_GetError());
        return -1;
    }

    auto* player_texture = load_texture(renderer,
                                        "/home/dwalker0044/Projects/ShapeAttack/res/player.png");

    DevHud dev_hud;
    auto*  dev_hud_texture = SDL_CreateTexture(renderer,
                                              SDL_PIXELFORMAT_ABGR8888,
                                              SDL_TEXTUREACCESS_TARGET,
                                              kiss_screen_width,
                                              kiss_screen_height);
    SDL_SetTextureBlendMode(dev_hud_texture, SDL_BLENDMODE_BLEND);

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
            // button_event(&dev_hud.button, &e, &draw, &quit);
            handle_input(e, game_events);
            dev_hud.handle_events(&e, &draw, game_events);
        }

        // Update gameplay.
        {
            player.update(dt, game_events.player_movement);
        }

        // Render gameplay.
        {
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
            SDL_Rect src{0, 0, 80, 80};
            SDL_Rect dst{(int)player.X[0][0], (int)to_screen_y(player.X[0][1] + 80), 80, 80};
            SDL_RenderCopy(renderer,
                           player_texture,
                           &src,
                           &dst);

            if (game_events.draw_vectors)
            {
                draw_velocity_vector(renderer, player);
            }
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