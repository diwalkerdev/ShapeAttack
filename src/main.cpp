#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm>
#include <complex>
#include <iostream>
#include <vector>

#include "screen.h"

#include "linalg/matrix.h"
#include "linalg/util.h"

constexpr static const auto screen = Screen();

template <typename Scalar, size_t NumSegments>
static linalg::Matrix<Scalar, NumSegments + 2, 3> make_circle_points(float radius)
{
    static_assert(NumSegments > 2);
    constexpr float seg_size = (2 * M_PI / NumSegments);

    linalg::Matrix<Scalar, NumSegments + 2, 3> points;

    float theta = 0;

    for (int i = 0; i < NumSegments + 1; ++i)
    {
        points[i][0] = cos(theta) * radius;
        points[i][1] = sin(theta) * radius;
        points[i][2] = 1;
        theta += seg_size;
    }

    int k = NumSegments + 1;

    points[k][0] = 0;
    points[k][1] = 0;
    points[k][2] = 1;

    return points;
}

// clang-format off
static linalg::Matrix<float, 5, 3> points {{
      0,  20, 1, 
     20, -20, 1, 
    -20, -20, 1, 
      0,  20, 1, 
      0,   0, 1
}};

static linalg::Matrix<int, 5, 2> square {{
     0,  0, 
     0, 40, 
    40, 40, 
    40,  0, 
     0,  0
}};
// clang-format on

static auto circle = make_circle_points<float, 5>(20);

static float turn      = 0;
static bool  quit_game = false;

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

#include <vector>


template <size_t NumSegs>
auto make_grid() -> std::vector<decltype(make_circle_points<float, NumSegs>(5) * rtransf(0, 0, 0))>
{
    constexpr int xs = 11;
    constexpr int ys = 11;

    constexpr int num_rows = xs * ys;

    linalg::Matrixf<num_rows, 2> points;

    int row = 0;

    for (int i = 0; i < xs; ++i)
    {
        for (int k = 0; k < ys; ++k)
        {
            points[row][0] = float(k);
            points[row][1] = float(i);
            // points[row][2] = float(1);
            ++row;
        }
    }

    // TODO: implement scalar multiplication.
    auto new_points(points * 50);
    std::cout << new_points;

    std::vector<decltype(make_circle_points<float, NumSegs>(5) * rtransf(0, 0, 0))> circles;

    for (int i = 0; i < num_rows; ++i)
    {
        float x = new_points[i][0];
        float y = new_points[i][1];

        auto circle = make_circle_points<float, NumSegs>(10);
        auto transc = circle * rtransf(0, x, y);

        std::cout << circle << "\n";
        std::cout << transc << "\n";

        circles.push_back(transc);
    }


    return circles;
}


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
                                    screen.width,
                                    screen.height,
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
    SDL_Rect background_rect {0, 0, screen.width, screen.height};

    float alpha = 0;
    float omega = 0;
    float theta = 0;

    auto const& player = circle;

    auto grid = make_grid<8>();

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
        // std::cout << alpha << " " << turn << " " << fd << "\n";

        // Draw the center player
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        auto rotated = player * rtransf(float(theta), 320.f, 240.f);

        int new_player[player.NumRows * player.NumCols];

        for (int i = 0; i < player.Size; ++i)
        {
            float val     = rotated.data[i];
            new_player[i] = int(val);
        }

        SDL_RenderDrawLines(
            renderer, (SDL_Point*)(&new_player[0]), player.NumRows);
        // SDL_RenderPresent(renderer);

        SDL_RenderDrawLines(
            renderer, (SDL_Point*)(&square.data[0]), square.NumRows);
        // SDL_RenderPresent(renderer);

        for (auto& grid_point : grid)
        {
            int new_point[grid_point.Size];
            for (int i = 0; i < grid_point.Size; ++i)
            {
                float val    = grid_point.data[i];
                new_point[i] = int(val);
            }
            SDL_RenderDrawLines(
                renderer, (SDL_Point*)(&new_point[0]), grid_point.NumRows);
            // SDL_RenderPresent(renderer);
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
