#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "devhud.h"
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

struct GameEvents {
    int                quit = 0;
    int                hud  = 0;
    int                l = 0, r = 0, u = 0, d = 0;
    linalg::Vectorf<2> player_movement;
};

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

    float x_axis = -game_events.l + game_events.r;
    float y_axis = -game_events.d + game_events.u;

    game_events.player_movement = {{x_axis, y_axis}};
}

auto to_screen_y(float y) -> float
{
    return SCREEN_HEIGHT - y;
}

auto to_screen_y(int y) -> int
{
    return SCREEN_HEIGHT - y;
}

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

//////////////////////////////////////////////////////////////////////////////

template <typename L, typename R>
auto player_update_physics(L& X, R& Xdot)
{
    float u  = turn * 5;
    float dt = 1 / 30.f;
    float m  = 1.f;
    // float k  = 0.2f;
    float v = X[1];
    float theta;

    // 0.2 tanh(10x)+x^3/10;
    linalg::Matrixf<2, 3> A{{{0, 1, 0},
                             {0, (-powf(v, 2) / 10.f * m), -tanhf(10 * v)}}};
    linalg::Matrixf<2, 1> B{{0, 1}};

    linalg::Matrixf<3, 1> Xn{{X[0], X[1], 1}};

    Xdot = (X + (dt * A * Xn)) + ((dt * B) * u);

    linalg::Matrixf<1, 2> C{{1, 0}};
    linalg::Matrixf<1, 1> y = C * X;

    X     = Xdot;
    theta = y[0];

    return -theta;
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
    SDL_Renderer* renderer;
    SDL_Event     e;
    kiss_array    objects;
    GameEvents    game_events;
    GameLoopTimer game_loop{0};
    int           draw;

    std::string window_title("Hello kiss_sdl");
    kiss_array_new(&objects);
    renderer = kiss_init(window_title.c_str(),
                         &objects,
                         640,
                         320);

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
        }

        // Render gameplay.
        {
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
            SDL_Rect box{0, 0, 50, 50};
            SDL_RenderFillRect(renderer, &box);

            SDL_Rect pbox{0, 0, 80, 80};
            SDL_RenderCopy(renderer,
                           player_texture,
                           &pbox,
                           &pbox);
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