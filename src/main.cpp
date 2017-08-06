#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <complex>
#include <iostream>
#include <vector>
#include <algorithm>

#include "screen.h"

constexpr static const auto screen = Screen();

constexpr const auto pi = 3.141592;
constexpr const auto radians = pi / 180.0;

struct GameRect {
  double x, y;
  double theta;
  SDL_Rect rect;
};

auto game_rects = std::vector<GameRect>(5);
auto bullets = std::vector<GameRect>(5);
auto bullet_visibility = std::vector<int>(5);

bool is_collision(SDL_Rect& rect1, SDL_Rect& rect2) {
  return (rect1.x < rect2.x + rect2.w && rect1.x + rect1.w > rect2.x &&
          rect1.y < rect2.y + rect2.h && rect1.h + rect1.y > rect2.y);
}

void init_attacking_shapes(GameRect& rect) {
  auto angle = (rand() % 10) *
               36.0;  // 10 starting points about the circle seems enough.
  auto theta = angle * radians;

  auto r = 200.0;
  auto cart = std::polar(r, theta);
  auto x = screen.center.x + cart.real() - 16.0;
  auto y = screen.center.y + cart.imag() - 16.0;

  auto itheta = atan2(screen.center.y - 16 - y, screen.center.x - 16 - x);
  if (itheta < 0.0) {
    itheta += (2 * pi);
  }

  rect.x = x;
  rect.y = y;
  rect.theta = itheta;
  rect.rect.w = 32;
  rect.rect.h = 32;
  rect.rect.x = x;
  rect.rect.y = y;
}

void init_bullets(GameRect& rect, int origin_x, int origin_y) {
  rect.x = origin_x;
  rect.y = origin_y;
  rect.theta = 0;
  rect.rect.w = 16;
  rect.rect.h = 16;
  rect.rect.x = origin_x;
  rect.rect.y = origin_y;
}

void handle_player_input() {
  SDL_Event event;

  SDL_PollEvent(&event);
  switch (event.type) {
    case SDL_KEYDOWN: {
      std::cout << "Got keydown event." << std::endl;
      if (event.key.keysym.sym == SDLK_SPACE) {
        std::cout << "Fire!!! Peow Peow!\n" << std::endl;
        auto it = std::find(
            std::begin(bullet_visibility), std::end(bullet_visibility), 0);
        if (it == std::end(bullet_visibility)) {
          printf("No more bullets");
        } else {
          // The bullet is now visible.
          *it = 1;
        }
      }
    }
  }
}

int main() {
  srand(time(nullptr));

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
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

  if (window == nullptr) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  // Create renderer
  auto* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  // Init game objects
  auto background_rect = SDL_Rect{0, 0, screen.width, screen.height};

  for (auto& r : game_rects) {
    init_attacking_shapes(r);
  }

  for (auto& v : bullet_visibility) {
    v = 0;
  }

  for (auto& b : bullets) {
    init_bullets(b, screen.center.x - 8, screen.center.y - 8);
  }

  int x = screen.center.x - 16;
  int y = screen.center.y - 16;
  auto player = SDL_Rect{x, y, 32, 32};

  for (int i = 0; i < 300; i++) {
    handle_player_input();
    fflush(stdout);

    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, &background_rect);

    // Draw the center player
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderFillRect(renderer, &player);

    // Update attacking objects
    for (auto& rect : game_rects) {
      const auto dx = 1.0 * cos(rect.theta);
      const auto dy = 1.0 * sin(rect.theta);
      rect.x += dx;
      rect.y += dy;
      rect.rect.x = static_cast<int>(rect.x);
      rect.rect.y = static_cast<int>(rect.y);

      // if (is_collision(rect.rect, player)) {
      //   std::cout << "!!!" << std::endl;
      // }

      SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
      SDL_RenderFillRect(renderer, &rect.rect);
    }

    for (int v = 0; v < 5; ++v) {
      if (bullet_visibility[v] == 1) {
        auto& b = bullets[v];

        const auto dx = 1.0 * cos(b.theta);
        const auto dy = 1.0 * sin(b.theta);
        b.x += dx;
        b.y += dy;
        b.rect.x = static_cast<int>(b.x);
        b.rect.y = static_cast<int>(b.y);

        // if (is_collision(rect.rect, player)) {
        //   std::cout << "!!!" << std::endl;
        // }

        SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
        SDL_RenderFillRect(renderer, &b.rect);
      }
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
