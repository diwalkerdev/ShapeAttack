#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fmt/core.h"
#include "kiss_sdl.h"
#include "linalg/matrix.hpp"
#include "linalg/misc.hpp"
#include "linalg/trans.hpp"
#include "misc.hpp"
#include "shapes.hpp"
#include "typedefs.h"
// #include "spdlog/spdlog.h"

#include <algorithm>
#include <complex>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

static constexpr int SCREEN_WIDTH       = 640;
static constexpr int SCREEN_HEIGHT      = 400;
static constexpr int HALF_SCREEN_WIDTH  = SCREEN_WIDTH / 2;
static constexpr int HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2;

static float turn = 0;
static bool  fire = false;

//////////////////////////////////////////////////////////////////////////////

void handle_input(SDL_Event& event, int& quit, int& hud_enabled)
{
    static int fire_debounce = 0;
    fire                     = false;


    if (event.type == SDL_KEYUP)
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
        case SDLK_SPACE:
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
        case SDLK_SPACE:
            if (fire_debounce == 0)
            {
                fire_debounce = 2;
                fire          = true;
            }

            break;

        case SDLK_h:
            hud_enabled = hud_enabled ? 0 : 1;
            break;
        case SDLK_ESCAPE:
            quit = 1;
            break;
        default:
            break;
        }
    }

    if (fire_debounce > 0)
    {
        --fire_debounce;
    }
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
    // float k  = 0.2f;
    float v = X[1];
    float theta;

    // 0.2 tanh(10x)+x^3/10;
    linalg::Matrixf<2, 3> A{{{0, 1, 0},
                             {0, (-powf(v, 2) / 10.f * m), -tanhf(10 * v)}}};
    linalg::Matrixf<2, 1> B{{0, 1}};

    linalg::Matrixf<3, 1> Xn{{X[0], X[1], 1}};

    Xdot = (X + (dt * A * Xn)) + ((dt * B) * u);

    linalg::Matrixf<1, 2> C{{1, 0}};
    linalg::Matrixf<1, 1> y = C * X;

    X     = Xdot;
    theta = y[0];

    return -theta;
}

//////////////////////////////////////////////////////////////////////////////
int tests();
#include "devhud.h"

void button_event(kiss_button* button, SDL_Event* e, int* draw, int* quit)
{
    if (kiss_button_event(button, e, draw))
    {
        printf("button event\n");
        *quit = 1;
    }
}


struct GameLoop {
    int32 start_frame, end_frame, time_taken, delay_time;
    float fps;

    void start()
    {
        start_frame = SDL_GetTicks();
    }

    void end()
    {
        end_frame  = SDL_GetTicks();
        time_taken = end_frame - start_frame;

        if (time_taken < 33)
        {
            delay_time = 33 - time_taken;
        }
        else
        {
            printf("WARNING FRAME TOOK LONGER THAN 33ms");
            delay_time = 0;
        }

        fps = 1000.0 / float(time_taken + delay_time);
    }

    void delay()
    {
        SDL_Delay(delay_time);
    }
};


int main()
{
    SDL_Renderer* renderer;
    SDL_Event     e;
    kiss_array    objects;
    kiss_label    label = {0};
    int           draw, quit, hud_enabled;
    quit        = 0;
    draw        = 1;
    hud_enabled = 0;
    GameLoop game_loop{0};

    kiss_array_new(&objects);

    std::string window_title("Hello kiss_sdl");
    renderer = kiss_init(window_title.c_str(),
                         &objects,
                         640,
                         320);

    if (renderer == nullptr)
    {
        printf("Renderer could not be created! SDL_Error: %s\n",
               SDL_GetError());
        return -1;
    }

    DevHud dev_hud;
    auto*  dev_hud_texture = SDL_CreateTexture(renderer,
                                              SDL_PIXELFORMAT_ABGR8888,
                                              SDL_TEXTUREACCESS_TARGET,
                                              kiss_screen_width,
                                              kiss_screen_height);
    SDL_SetTextureBlendMode(dev_hud_texture, SDL_BLENDMODE_BLEND);

    while (!quit)
    {
        game_loop.start();

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
            kiss_window_event(&dev_hud.window, &e, &draw);
            // button_event(&dev_hud.button, &e, &draw, &quit);
            handle_input(e, quit, hud_enabled);
        }

        // TODO: this doesn't do what you think it does.
        // if (!draw)
        // {
        //     printf("Don't draw\n");
        //     continue;
        // }

        // Render gameplay.
        {
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
            SDL_Rect box{0, 0, 50, 50};
            SDL_RenderFillRect(renderer, &box);
        }

        // Render hud. This is a bit wasteful if the hud is not enabled
        // however for not it is good to do so we can keep an eye on CPU usage.
        {
            dev_hud.update(game_loop.time_taken,
                           game_loop.fps);
            dev_hud.render(renderer, dev_hud_texture);
        }

        // Copy textures from kiss to the screen.
        {
            if (hud_enabled)
            {
                SDL_RenderCopy(renderer,
                               dev_hud_texture,
                               &dev_hud.window.rect,
                               &dev_hud.window.rect);
            }

            SDL_RenderPresent(renderer);
        }
        draw = 0;

        game_loop.end();
        game_loop.delay();
    }

    kiss_clean(&objects);
}

int other()
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
    SDL_Rect background_rect{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

    auto player = Shape<3>(20, HALF_SCREEN_WIDTH, HALF_SCREEN_HEIGHT);

    std::array<Bullet, 3> bullets;
    for (auto& bullet : bullets)
    {
        bullet = Bullet(4);
    }
    int bindex = 0;

    linalg::Vectorf<2> X{{M_PI_2, 0}};
    linalg::Vectorf<2> Xdot{{0, 0}};

    int start_frame, end_frame, time_taken, delay_time;

    bool quit_game = true;
    while (!quit_game)
    {
        start_frame = SDL_GetTicks();
        // handle_input();

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(renderer, &background_rect);

        // Update the player.
        {
            float theta  = player_update_physics(X, Xdot);
            auto  points = player.data * rtransf(theta, player.x, player.y);
            player.theta = theta;

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
            draw(renderer, points);
        }

        // update the bullet.
        {
            if (fire)
            {
                if (bindex < 3)
                {
                    auto& bullet = bullets[bindex];
                    bullet.fire(player.theta, {{HALF_SCREEN_WIDTH, HALF_SCREEN_HEIGHT}});
                    bindex += 1;
                }
                else
                {
                    // spdlog::info("No more bullets.");
                }
            }
        }

        assert(bindex <= 3);
        for (int i = 0; i < bindex; ++i)
        {
            // spdlog::info("i: {0}  bindex: {1}", i, bindex);
            auto& bullet = bullets[i];
            auto  points = bullet.update();
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
            draw(renderer, points);

            auto is_active = bullet.check_collisions();
            if (!is_active)
            {
                bullet = bullets[bindex - 1];
                bindex -= 1;
            }
        };

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
