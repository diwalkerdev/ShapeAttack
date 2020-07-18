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

#define POINTS_COUNT 4



static linalg::Matrix<float, 4, 2> points{{320, 200,
                                         300, 240,
                                         340, 240,
                                         320, 200}};

static linalg::Matrix<int, 5, 2> square{{ 0,  0, 
                                          0, 40, 
                                         40, 40, 
                                         40,  0,
                                          0,  0}};


void handle_player_input()
{
    SDL_Event event;

    SDL_PollEvent(&event);
    switch (event.type)
    {
    case SDL_KEYDOWN:
    {
        if (event.key.keysym.sym == SDLK_SPACE)
        {

        }
    }
    }
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
    auto *window = SDL_CreateWindow("SDL Tutorial",
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
    auto *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        printf("Renderer could not be created! SDL_Error: %s\n",
               SDL_GetError());
        return -1;
    }

    // Init game objects
    SDL_Rect background_rect{0, 0, screen.width, screen.height};

    for (int i = 0; i < 300; i++)
    {
        handle_player_input();

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(renderer, &background_rect);

        // Draw the center player
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        auto rotated = points * rrotzf(float(2*M_PI*i / 180));
        std::cout << rotated;
        int new_points[points.NumRows * points.NumCols];

        for (int i=0; i<points.Size; ++i){
            float val = rotated.data[i];
            new_points[i] = int(val);
        }

        SDL_RenderDrawLines(renderer, (SDL_Point*)(&new_points[0]), points.NumRows);
        SDL_RenderPresent(renderer);

        SDL_RenderDrawLines(renderer, (SDL_Point*)(&square.data[0]), square.NumRows);
        SDL_RenderPresent(renderer);

        // Update the screen with rendering actions
        SDL_RenderPresent(renderer);

        SDL_Delay(50);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
