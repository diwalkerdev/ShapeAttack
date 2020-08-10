#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "linalg/matrix.hpp"
#include "linalg/misc.hpp"
#include "linalg/trans.hpp"
#include "shapes.hpp"
#include "transformations.hpp"

#include <algorithm>
#include <complex>
#include <iostream>
#include <numeric>
#include <vector>

static constexpr int SCREEN_WIDTH  = 640;
static constexpr int SCREEN_HEIGHT = 400;

static auto circle = make_circle<12>(20);

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
auto make_grid_points(float scale) -> std::vector<linalg::Matrixf<NumSegs + 2, 3>>
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

    points *= scale;

    std::vector<linalg::Matrixf<NumSegs + 2, 3>> circles;

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

template <size_t Xs, size_t Ys>
auto make_grid_points(float scale) -> std::vector<linalg::Matrixf<8 + 2, 2>>
{
    return make_grid_points<Xs, Ys, 8>(scale);
}

//////////////////////////////////////////////////////////////////////////////

template <int NumLines>
auto make_grid_lines(float scale) -> linalg::Matrixf<(NumLines + 1) * 2, 6>
{
    static_assert((NumLines % 2) == 0);

    linalg::Matrixf<(NumLines + 1) * 2, 6> grid;

    const float length = NumLines;

    // Vertical lines
    for (auto i : irange<NumLines + 1>())
    {
        float fi = (float)i;
        span_deepcopy(grid[i], {0.f, fi, 1.f, length, fi, 1.f});
    }

    // Horizontal lines
    for (auto i : irange<NumLines + 1>())
    {
        float fi = (float)i;
        span_deepcopy(grid[i + NumLines + 1], {fi, 0.f, 1.f, fi, length, 1.f});
    }

    // Here we are Euler coordinates. Center the grid about (0,0).
    return grid * grid_tmatt(0, -NumLines / 2.f, -NumLines / 2.f);
}

//////////////////////////////////////////////////////////////////////////////
auto to_screen_y(float y) -> float
{
    return SCREEN_HEIGHT - y;
}

auto to_screen_y(int y) -> int
{
    return SCREEN_HEIGHT - y;
}

int main()
{
    // clang-format off
    linalg::Matrixf<3, 3> A {{
        1.f, 0.f, 7.f,
        0.f, 1.f, 1.f,
        0.f, 1.f, 2.f,
    }};
    // clang-format on

    auto new_mat = linalg::cols(A, {0, 2, 2, 1});
    std::cout << new_mat;

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

    const float scale      = 40;
    const int   NumLines   = 8;
    auto        grid_lines = make_grid_lines<NumLines>(scale);

    auto player = circle * rtransf(0, scale, scale);

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

        // Draw the grid.
        {
            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
            float offset_x     = SCREEN_WIDTH / 2 / scale;
            float offset_y     = SCREEN_HEIGHT / 2 / scale;
            auto  rotated_grid = grid_lines * grid_tmatt(theta, offset_x, offset_y) * scale;

            for (auto r : iter(rotated_grid))
            {
                SDL_RenderDrawLineF(renderer,
                                    r[0],
                                    to_screen_y(r[1]),
                                    r[3],
                                    to_screen_y(r[4]));
            }
        }

        // Draw the basis point.
        {
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
            float offset_x = (SCREEN_WIDTH / 2);
            float offset_y = (SCREEN_HEIGHT / 2);

            auto rotated = player * rtransf(float(theta), offset_x, offset_y);

            for (auto r : iter(rotated))
            {
                SDL_RenderDrawLineF(renderer,
                                    r[0],
                                    to_screen_y(r[1]),
                                    r[3],
                                    to_screen_y(r[4]));
            }
            // SDL_RenderDrawLinesF(renderer,
            //                      (SDL_FPoint*)(&rotated.data[0]),
            //                      player.NumRows);
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
