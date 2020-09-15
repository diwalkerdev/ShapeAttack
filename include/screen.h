#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

static constexpr int SCREEN_WIDTH       = 640;
static constexpr int SCREEN_HEIGHT      = 400;
static constexpr int HALF_SCREEN_WIDTH  = SCREEN_WIDTH / 2;
static constexpr int HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2;

constexpr auto to_screen_y(float y) -> float
{
    return SCREEN_HEIGHT - y;
}

constexpr auto to_screen_y(int y) -> int
{
    return SCREEN_HEIGHT - y;
}

#endif // GAME_SCREEN_H