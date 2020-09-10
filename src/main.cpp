#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fmt/core.h"
#include "linalg/matrix.hpp"
#include "linalg/misc.hpp"
#include "linalg/trans.hpp"
#include "misc.hpp"
#include "shapes.hpp"
// #include "spdlog/spdlog.h"
#include "typedefs.h"

#include <algorithm>
#include <complex>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

static constexpr int SCREEN_WIDTH       = 640;
static constexpr int SCREEN_HEIGHT      = 400;
static constexpr int HALF_SCREEN_WIDTH  = SCREEN_WIDTH / 2;
static constexpr int HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2;

static float turn             = 0;
static bool  quit_game        = false;
static bool  fire             = false;
static bool  heads_up_display = false;

//////////////////////////////////////////////////////////////////////////////

void handle_input(SDL_Event& event)
{
    static int fire_debounce = 0;
    fire                     = false;

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
        case SDLK_SPACE:
            break;
        case SDLK_h:
            heads_up_display = false;
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
        case SDLK_ESCAPE:
            quit_game = true;
            break;
        case SDLK_h:
            heads_up_display = true;
            printf("hud\n");
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
#include "kiss_sdl.h"

void button_event(kiss_button* button, SDL_Event* e, int* draw, int* quit)
{
    if (kiss_button_event(button, e, draw))
        *quit = 1;
}


int main()
{

    SDL_Renderer* renderer;
    SDL_Event     e;
    kiss_array    objects;
    kiss_window   window;
    kiss_label    label  = {0};
    kiss_button   button = {0};
    char          message[KISS_MAX_LENGTH];
    int           draw, quit;
    quit = 0;
    draw = 1;

    kiss_array_new(&objects);
    renderer = kiss_init("Hello kiss_sdl", &objects, 640, 320);
    if (!renderer)
    {
        return 1;
    }

    kiss_window_new(&window, NULL, 0, 0, 0, kiss_screen_width, kiss_screen_height);
    window.bg = {0xff, 0xff, 0xff, 0x70};

    strcpy(message, "Hello World!");

    kiss_label_new(&label,
                   &window,
                   message,
                   (window.rect.w / 2) - strlen(message) * kiss_textfont.advance / 2,
                   (window.rect.h / 2) - (kiss_textfont.fontheight + 2 * kiss_normal.h) / 2);
    label.textcolor.r = 255;
    kiss_button_new(&button,
                    &window,
                    "OK",
                    (window.rect.w / 2) - kiss_normal.w / 2,
                    label.rect.y + kiss_textfont.fontheight + kiss_normal.h);
    window.visible = 1;

    auto* hud = SDL_CreateTexture(renderer,
                                  SDL_PIXELFORMAT_ABGR8888,
                                  SDL_TEXTUREACCESS_TARGET,
                                  window.rect.w,
                                  window.rect.h);
    SDL_SetTextureBlendMode(hud, SDL_BLENDMODE_BLEND);

    while (!quit)
    {
        SDL_Delay(10);
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
            kiss_window_event(&window, &e, &draw);
            button_event(&button, &e, &draw, &quit);
            handle_input(e);
        }

        {
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
            SDL_Rect box{0, 0, 50, 50};
            SDL_RenderFillRect(renderer, &box);
        }

        SDL_SetRenderTarget(renderer, hud);
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
        SDL_RenderClear(renderer);
        if (heads_up_display)
        {
            kiss_window_draw(&window, renderer);
            kiss_label_draw(&label, renderer);
            kiss_button_draw(&button, renderer);
        }

        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderCopy(renderer, hud, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        draw = 0;
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

int tests()
{
    const auto               A = linalg::Matrixf<2, 2>{{{1, 2}, {3, 4}}};
    const auto               B = linalg::Matrixf<2, 2>{{{1, 2}, {3, 4}}};
    const linalg::Vectorf<2> v{{1, 2}};

    // std::cout << "A: " << A;
    // std::cout << "v: " << v;
    // std::cout << "rvalue: " << linalg::Matrixf<2, 2>{{{1, 2}, {3, 4}}};

    fmt::print("A: {0}\n", A);
    fmt::print("v: {0}\n", v);
    fmt::print("rvalue: {0}", linalg::Matrixf<2, 2>{{{1, 2}, {3, 4}}});


    auto X = linalg::Matrixf<2, 2>{{{1, 2}, {3, 4}}};
    // fmt::print("rows {}  cols {}\n", X.rows(), X.cols());

    // Check can access const matrices.
    float x = A[0][0];

    //A[0] = 2; // cc error, A is const.

    {
        auto C = A * B;
        auto D = C * 2.f;
        auto E = 2.f * C;
        // E *= A; // currently undefined, ambiguous? do you point-wise or matrix mult?
        D *= 2.f;

        assert(A == A);

        // fmt::print("{0}\n", A);
    }

    {
        auto C = A + B;
        auto D = C + 2.f;
        auto E = 2.f + C;

        E += E += A;
        std::cout << C;
        std::cout << D;
        std::cout << E;
        D += 2.f;
        std::cout << D;
    }

    return 0;
}