#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "linalg/matrix.h"
#include "linalg/util.h"
#include "misc.hpp"
#include "shapes.hpp"
#include "transformations.hpp"

#include <algorithm>
#include <complex>
#include <iostream>
#include <numeric>
#include <vector>

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 400;

static auto  circle    = make_circle<float, 7>(40);
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
                turn = -1;
                break;
            case SDLK_RIGHT:
                turn = 1;
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

//////////////////////////////////////////////////////////////////////////////

template <size_t Xs, size_t Ys, size_t NumSegs>
auto make_grid_points() -> std::vector<linalg::Matrixf<NumSegs + 2, 2>>
{
    constexpr int NumRows = Xs * Ys;

    linalg::Matrixf<NumRows, 2> points;

    int row = 0;

    for (auto i : irange<Xs>())
    {
        for (auto k : irange<Ys>())
        {
            points[row][0] = float(k);
            points[row][1] = float(i);
            ++row;
        }
    }

    points *= 50;

    std::vector<linalg::Matrixf<NumSegs + 2, 2>> circles;

    for (int i = 0; i < NumRows; ++i)
    {
        float x = points[i][0];
        float y = points[i][1];

        auto circle = make_circle_points<float, NumSegs>(10);
        auto transc = circle * rtransf(0, x, y);

        circles.push_back(transc);
    }

    return circles;
}

//////////////////////////////////////////////////////////////////////////////

template <int NumLines>
auto make_grid_lines(float scale) -> linalg::Matrixf<NumLines * 2, 6>
{
    linalg::Matrixf<NumLines * 2, 6> grid;

    float length = NumLines;

    for (auto i : irange<NumLines>())
    {
        span_deepcopy(grid[i], {0.f, (float)i, 1.f, length, (float)i, 1.f});
    }

    for (auto i : irange<NumLines>())
    {
        span_deepcopy(grid[i + NumLines], {(float)i, 0.f, 1.f, (float)i, length, 1.f});
    }


    return grid * grid_tmatt(0, -NumLines / 2.f, -NumLines / 2.f);
}


template <size_t Xs, size_t Ys>
auto make_grid_points() -> std::vector<linalg::Matrixf<8 + 2, 2>>
{
    return make_grid_points<Xs, Ys, 8>();
}

//////////////////////////////////////////////////////////////////////////////

int main()
{
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
    SDL_Rect background_rect {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

    float alpha = 0;
    float omega = 0;
    float theta = 0;

    auto& player = circle;

    const int grid_rows = 11;
    const int grid_cols = 11;
    auto      grid      = make_grid_points<grid_rows, grid_cols>();

    const float scale      = 40;
    const int   NumLines   = 11;
    auto        grid_lines = make_grid_lines<NumLines>(scale);

    while (!quit_game)
    {
        handle_input();

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(renderer, &background_rect);

        float dt = 1 / 30.f;
        float A  = 0.2;
        float fd = A * pow(omega, 2);
        float d  = omega > 0 ? -1 : 1;
        alpha    = (turn * 20) + (fd * d);
        omega    = omega + (alpha * dt);
        theta    = theta + (omega * dt);

        // Draw the center player
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        auto rotated = player * rtransf(float(theta), 320.f, 240.f);

        SDL_RenderDrawLinesF(
            renderer, (SDL_FPoint*)(&rotated.data[0]), player.NumRows);

        for (auto& grid_point : grid)
        {
            SDL_RenderDrawLinesF(
                renderer, (SDL_FPoint*)(&grid_point.data[0]), grid_point.NumRows);
        }

        auto rotated_grid = grid_lines * grid_tmat(theta, +NumLines / 2.f, +NumLines / 2.f);
        rotated_grid *= scale;

        for (auto r : iter(rotated_grid))
        {
            SDL_RenderDrawLineF(renderer, r[0], r[1], r[2], r[3]);
        }

        // Update the screen with rendering actions
        SDL_RenderPresent(renderer);

        SDL_Delay(33);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
