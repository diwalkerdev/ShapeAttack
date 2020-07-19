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
        std::cout << alpha << " " << turn << " " << fd << "\n";

        // Draw the center player
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        auto rotated = points * rtransf(float(theta), 320.f, 240.f);

        int new_points[points.NumRows * points.NumCols];

        for (int i = 0; i < points.Size; ++i)
        {
            float val     = rotated.data[i];
            new_points[i] = int(val);
        }

        SDL_RenderDrawLines(
            renderer, (SDL_Point*)(&new_points[0]), points.NumRows);
        SDL_RenderPresent(renderer);

        SDL_RenderDrawLines(
            renderer, (SDL_Point*)(&square.data[0]), square.NumRows);
        SDL_RenderPresent(renderer);

        // Update the screen with rendering actions
        SDL_RenderPresent(renderer);

        SDL_Delay(33);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
