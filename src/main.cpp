#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fmt/core.h"
#include "linalg/matrix.hpp"
#include "linalg/misc.hpp"
#include "linalg/trans.hpp"
#include "shapes.hpp"
#include "spdlog/spdlog.h"
#include "typedefs.h"

#include <algorithm>
#include <complex>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

static constexpr int SCREEN_WIDTH       = 640;
static constexpr int SCREEN_HEIGHT      = 400;
static constexpr int HALF_SCREEN_WIDTH  = SCREEN_WIDTH / 2;
static constexpr int HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2;

static float turn      = 0;
static bool  quit_game = false;

//////////////////////////////////////////////////////////////////////////////

void handle_input()
{
    SDL_Event event;

    while (SDL_PollEvent(&event) != 0)
    {
        if (event.type == SDL_QUIT)
        {
        }
        else if (event.type == SDL_KEYUP)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
                break;
            case SDLK_DOWN:
                break;
            case SDLK_LEFT:
                turn = 0;
                break;
            case SDLK_RIGHT:
                turn = 0;
                break;
            case SDLK_LSHIFT:
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
                break;
            case SDLK_DOWN:
                break;
            case SDLK_LEFT:
                turn = 1;
                break;
            case SDLK_RIGHT:
                turn = -1;
                break;
            case SDLK_LSHIFT:
                break;
            case SDLK_ESCAPE:
                quit_game = true;
                break;
            default:
                break;
            }
        }
    } // End event loop
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
    float k  = 0.2f;
    float v  = X[1];
    float theta;

    // 0.2 tanh(10x)+x^3/10;
    linalg::Matrixf<2, 3> A{{{0, 1, 0},
                             {0, (-powf(v, 2) / 10.f * m), -tanhf(10 * v)}}};
    linalg::Matrixf<2, 1> B{{{0, 1}}};

    linalg::Matrixf<3, 1> Xn{{{X[0], X[1], 1}}};

    Xdot = (X + (dt * A * Xn)) + ((dt * B) * u);

    linalg::Matrixf<1, 2> C{{{1, 0}}};
    linalg::Matrixf<1, 1> y = C * X;

    X     = Xdot;
    theta = y[0];

    return -theta;
}

//////////////////////////////////////////////////////////////////////////////

int main()
{
    spdlog::info("Starting Shape Attack {}", 42);
    fmt::print("The answer is {}.\n", 42);
    srand(time(nullptr));

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Failed to initialize.");
        return -1;
    }

    // Create window
    auto* window = SDL_CreateWindow("SDL Tutorial",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SCREEN_WIDTH,
                                    SCREEN_HEIGHT,
                                    SDL_WINDOW_SHOWN);

    if (window == nullptr)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create renderer
    auto* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        printf("Renderer could not be created! SDL_Error: %s\n",
               SDL_GetError());
        return -1;
    }

    // Init game objects
    SDL_Rect background_rect{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

    auto player = Shape<3>(20, HALF_SCREEN_WIDTH, HALF_SCREEN_HEIGHT);

    linalg::Vectorf<2> X{{M_PI_2, 0}};
    linalg::Vectorf<2> Xdot{{0, 0}};

    int start_frame, end_frame, time_taken, delay_time;

    while (!quit_game)
    {
        start_frame = SDL_GetTicks();
        handle_input();

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(renderer, &background_rect);

        // Update the player.
        {
            float theta  = player_update_physics(X, Xdot);
            auto  points = player.data * rtransf(theta, player.x, player.y);

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
            draw(renderer, points);
        }

        end_frame  = SDL_GetTicks();
        time_taken = end_frame - start_frame;

        delay_time = 0;
        if (time_taken < 33)
        {
            delay_time = 33 - time_taken;
        }
        else
        {
            printf("WARNING FRAME TOOK LONGER THAN 33ms");
        }

        SDL_Delay(delay_time);

        // Update the screen with rendering actions
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
