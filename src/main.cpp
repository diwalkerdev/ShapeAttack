#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <complex>
#include <iostream>
#include <vector>

#include "screen.h"

constexpr static const auto screen = Screen();

constexpr const auto pi = 3.141592;
constexpr const auto radians = pi / 180.0;


struct GameRect {
  double x, y;
  double theta;
  SDL_Rect rect;
};

bool is_collision(SDL_Rect& rect1, SDL_Rect& rect2) {
  return (rect1.x < rect2.x + rect2.w &&
   rect1.x + rect1.w > rect2.x &&
   rect1.y < rect2.y + rect2.h &&
   rect1.h + rect1.y > rect2.y);
}


void init_game_rect(GameRect& rect) {
  auto angle =
      (rand() % 10) * 36.0; // 10 starting points about the circle seems enough.
  auto theta = angle * radians;
  
  auto r = 200.0;
  auto cart = std::polar(r, theta);
  auto x = screen.center.x + cart.real() - 16.0;
  auto y = screen.center.y + cart.imag() - 16.0;

  auto itheta = atan2(screen.center.y - 16 - y, screen.center.x - 16 - x);
  if (itheta < 0.0) { itheta += (2*pi); }

  rect.x = x;
  rect.y = y;
  rect.theta = itheta;
  rect.rect.w = 32;
  rect.rect.h = 32;
  rect.rect.x = x;
  rect.rect.y = y;
}

int main() {
  srand(time(nullptr));

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Failed to initialize.");
    return -1;
  }

  // Create window
  auto *window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, screen.width,
                                  screen.height, SDL_WINDOW_SHOWN);

  if (window == nullptr) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  // Create renderer
  auto *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  // Init game objects
  auto background_rect = SDL_Rect{0, 0, screen.width, screen.height};

  auto game_rects = std::vector<GameRect>(5);
  for (auto &r : game_rects) {
    init_game_rect(r);
  }

  int x = screen.center.x - 16;
  int y = screen.center.y - 16;
  auto player = SDL_Rect{x, y, 32, 32};

  for (int i = 0; i < 300; i++) {
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, &background_rect);

    // Draw the center player
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderFillRect(renderer, &player);

    // Update attacking objects
    for (auto &rect : game_rects) {
      const auto dx = 1.0 * cos(rect.theta);
      const auto dy = 1.0 * sin(rect.theta);
      rect.x += dx;
      rect.y += dy;
      rect.rect.x = rect.x;
      rect.rect.y = rect.y;

      if (is_collision(rect.rect, player)) {
        std::cout << "!!!" << std::endl;
      }

      SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
      SDL_RenderFillRect(renderer, &rect.rect);
    }

    // Update the screen with rendering actions
    SDL_RenderPresent(renderer);

    SDL_Delay(10);
  }

  // Destroy renderer
  SDL_DestroyRenderer(renderer);

  // Destroy window
  SDL_DestroyWindow(window);

  // Quit SDL subsystems
  SDL_Quit();

  return 0;
}

