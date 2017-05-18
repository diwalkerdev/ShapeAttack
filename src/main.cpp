#include <SDL.h>
#include <stdio.h>

#include <vector>
#include <complex>
#include <iostream>

constexpr const int SCREEN_WIDTH = 640;
constexpr const int SCREEN_HEIGHT = 480;
constexpr const int SCREEN_WIDTH_MID = SCREEN_WIDTH / 2;
constexpr const int SCREEN_HEIGHT_MID = SCREEN_HEIGHT / 2;
constexpr const auto pi = 3.141592;

struct Vector2d {
  int dx, dy;
};

struct GameRect {
  SDL_Rect rect;
  Vector2d vec;
};

GameRect make_game_rect() {
  static double angle = 0.0;
  auto theta = angle * (pi / 180.0);
  auto r = 32.0;
  auto cart = std::polar(r, theta);
  std::cout << cart << std::endl;
  angle += 90;
  auto x = static_cast<int>(cart.real() + SCREEN_WIDTH_MID);
  auto y = static_cast<int>(cart.imag() + SCREEN_HEIGHT_MID);
  return GameRect{{x, y, 32, 32}, {1, 0}};
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
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
  auto background_rect = SDL_Rect{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

  auto game_rects = std::vector<GameRect>(5);
  int count = 0;
  for (auto& r : game_rects) {
    r = make_game_rect();
    ++count;
  }

  for (int i = 0; i < 1; i++) {
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, &background_rect);

    // Update
    for (auto& rect : game_rects) {
      // rect.rect.x += 1;

      // Draw a red square
      SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
      SDL_RenderFillRect(renderer, &rect.rect);
    }

    // Update the screen with rendering actions
    SDL_RenderPresent(renderer);

    SDL_Delay(1000);
  }

  // Destroy renderer
  SDL_DestroyRenderer(renderer);

  // Destroy window
  SDL_DestroyWindow(window);

  // Quit SDL subsystems
  SDL_Quit();

  return 0;
}
