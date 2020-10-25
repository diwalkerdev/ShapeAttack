#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "animation/core.hpp"
#include "collision/core.hpp"
#include "containers/backfill_vector.hpp"
#include "drawing/core.hpp"
#include "entity/core.hpp"
#include "fmt/core.h"
#include "gameevents.h"
#include "gamehud.h"
#include "kiss_sdl.h"
#include "linalg/matrix.hpp"
#include "linalg/misc.hpp"
#include "linalg/trans.hpp"
#include "misc.hpp"
#include "propertyeditor.hpp"
#include "recthelper.hpp"
#include "screen.h"
#include "shapes.hpp"
#include "typedefs.h"

// #include "spdlog/spdlog.h"

#include <algorithm>
#include <complex>
#include <filesystem>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

void handle_input(SDL_Event& event, GameEvents& game_events)
{
    if (event.type == SDL_KEYUP)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
            game_events.u = 0;
            break;
        case SDLK_DOWN:
            game_events.d = 0;
            break;
        case SDLK_LEFT:
            game_events.l = 0;
            break;
        case SDLK_RIGHT:
            game_events.r = 0;
            break;
        case SDLK_a:
            game_events.player_rotation = 0;
            break;
        case SDLK_d:
            game_events.player_rotation = 0;
            break;
        case SDLK_LSHIFT:
            break;
        case SDLK_SPACE:
            game_events.fire = 0;
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
            game_events.u = 1;
            break;
        case SDLK_DOWN:
            game_events.d = 1;
            break;
        case SDLK_LEFT:
            game_events.l = 1;
            break;
        case SDLK_RIGHT:
            game_events.r = 1;
            break;
        case SDLK_a:
            game_events.player_rotation = -1;
            break;
        case SDLK_d:
            game_events.player_rotation = 1;
            break;
        case SDLK_LSHIFT:
            break;
        case SDLK_SPACE:
            game_events.fire = 1;
            break;
        case SDLK_h:
            game_events.hud = !game_events.hud;
            break;
        case SDLK_ESCAPE:
            game_events.quit = 1;
            break;
        default:
            break;
        }
    }

    {
        float x_axis = game_events.r - game_events.l;
        float y_axis = game_events.u - game_events.d;

        if (x_axis != 0 && y_axis != 0)
        {
            float norm = std::sqrt((x_axis * x_axis) + (y_axis * y_axis));
            x_axis /= norm;
            y_axis /= norm;
        }

        game_events.player_movement = {
            {{0.f, 0.f},
             {x_axis, y_axis}}};
    }
}

//////////////////////////////////////////////////////////////////////////////

struct GameLoopTimer {
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

///////////////////////////////////////////////////////////////////////////////

SDL_Texture* load_texture(SDL_Renderer* renderer,
                          std::string   path)
{
    // The final texture
    SDL_Texture* new_texture = NULL;

    // Load image at specified path
    SDL_Surface* loaded_surface = IMG_Load(path.c_str());
    if (loaded_surface == NULL)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n",
               path.c_str(),
               IMG_GetError());
        exit(-1);
    }

    // Create texture from surface pixels
    new_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    if (new_texture == NULL)
    {
        printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
    }

    printf("Loaded %s\n", path.c_str());

    // Get rid of old loaded surface
    SDL_FreeSurface(loaded_surface);

    return new_texture;
}

///////////////////////////////////////////////////////////////////////////////

void create_game_objects(linalg::Vectorf<2>                 origin,
                         std::vector<entity::EntityStatic>& game_entities,
                         std::vector<entity::EntityStatic>& walls,
                         std::vector<SDL_FRect>&            soft_boundaries,
                         std::vector<SDL_FRect>&            hard_boundaries)
{
    game_entities.push_back(entity::make_food());
    walls.push_back(entity::make_wall());

    for (entity::EntityStatic& entity : walls)
    {
        hard_boundaries.push_back(collision::minkowski_boundary(entity, origin));
    }
    for (entity::EntityStatic& entity : game_entities)
    {
        soft_boundaries.push_back(collision::minkowski_boundary(entity, origin));
    }
}

///////////////////////////////////////////////////////////////////////////////

namespace serialisation {
extern auto save(std::filesystem::path const&, GameEvents&) -> void;
extern auto load(std::filesystem::path const&, GameEvents&) -> void;
}

///////////////////////////////////////////////////////////////////////////////

void test_bf_vector()
{
    backfill_vector<int, 5> bfvec;
    println("size:", bfvec.size());

    int i = 0;
    bfvec.push_back(i++);
    bfvec.push_back(i++);
    bfvec.push_back(i++);
    bfvec.push_back(i++);
    bfvec.push_back(i++);

    println("size b4:", bfvec.size());
    bfvec.remove(4);
    println("size:", bfvec.size());
    bfvec.remove(2);
    println("size:", bfvec.size());
    bfvec.remove(0);
    println("size:", bfvec.size());

    for (auto x : bfvec)
    {
        println("item:", x);
    }
}

int main(int argc, char* argv[])
{
    test_bf_vector();
    std::string           argv_str(argv[0]);
    std::filesystem::path exe_base_dir(argv_str.substr(0, argv_str.find_last_of("/")));
    std::filesystem::path game_state_path = (exe_base_dir / "game_state.msgpack");

    constexpr float dt      = 1.f / 30.f;
    constexpr float dt_step = dt / 4.f;

    SDL_Renderer*      renderer;
    SDL_Event          e;
    kiss_array         objects;
    GameEvents         game_events;
    GameLoopTimer      game_loop{0};
    int                draw;
    auto               player = entity::make_player(100.f, 100.f, 80.f, 80.f);
    linalg::Vectorf<2> origin{{-80, -80}};

    std::vector<entity::EntityStatic> game_entities;
    std::vector<entity::EntityStatic> walls;
    std::vector<SDL_FRect>            soft_boundaries;
    std::vector<SDL_FRect>            hard_boundaries;

    if (std::filesystem::exists(game_state_path))
    {
        serialisation::load(game_state_path, game_events);
    }
    else
    {
        fmt::print("{0} does not exist.", game_state_path.c_str());
    }

    kiss_array_new(&objects);
    renderer = kiss_init("Hello kiss_sdl",
                         &objects,
                         SCREEN_WIDTH,
                         SCREEN_HEIGHT);

    // Note, you MUST create the hud after kiss_init.
    SDL_Rect screen_rect{0,
                         0,
                         kiss_screen_width,
                         kiss_screen_height};
    GameHud  game_hud;

    // TODO: Refactor DataStructure name
    // TODO: Refactor how the window, texture and grid are all created.
    DataStructure window_data(
        std::tuple{"FPS", &game_loop.fps},
        std::tuple{"Draw Minkowski", &game_events.draw_minkowski},
        std::tuple{"Show Vectors", &game_events.draw_vectors},
        std::tuple{"Player x", (const float*)&player.e.X[0][0]},
        std::tuple{"Player y", (const float*)&player.e.X[0][1]});

    kiss_window editor_window;
    kiss_window_new(&editor_window,
                    NULL,
                    1,
                    screen_rect.x,
                    screen_rect.y,
                    screen_rect.w / 2,
                    screen_rect.h);
    editor_window.bg      = {0x7f, 0x7f, 0x7f, 0x70};
    editor_window.visible = 1;

    Grid2x2 grid(&editor_window);
    window_init(window_data, &editor_window, grid);

    if (renderer == nullptr)
    {
        printf("Renderer could not be created! SDL_Error: %s\n",
               SDL_GetError());
        return -1;
    }

    player.texture = load_texture(renderer,
                                  "/home/dwalker0044/Projects/Untitled2D/res/lruptest.png");

    auto* dev_hud_texture = SDL_CreateTexture(renderer,
                                              SDL_PIXELFORMAT_ABGR8888,
                                              SDL_TEXTUREACCESS_TARGET,
                                              kiss_screen_width,
                                              kiss_screen_height);
    SDL_SetTextureBlendMode(dev_hud_texture, SDL_BLENDMODE_BLEND);


    auto* game_hud_texture = SDL_CreateTexture(renderer,
                                               SDL_PIXELFORMAT_ABGR8888,
                                               SDL_TEXTUREACCESS_TARGET,
                                               kiss_screen_width,
                                               kiss_screen_height);
    SDL_SetTextureBlendMode(game_hud_texture, SDL_BLENDMODE_BLEND);

    create_game_objects(origin,
                        game_entities,
                        walls,
                        soft_boundaries,
                        hard_boundaries);

    assert(game_entities.size() == soft_boundaries.size());
    assert(walls.size() == hard_boundaries.size());

    auto player_texture_descriptor = animation::make_LRUPDescriptor<2>(player.texture);
    int  accumilator               = 0;
    int  frame                     = 0;

    animation::Direction direction = animation::Direction::RIGHT;


    while (!game_events.quit)
    {
        game_loop.start();

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                game_events.quit = 1;
            }

            editor_handle_events(window_data, &e, &draw);
            game_hud.handle_events(&e, &draw, game_events);
            handle_input(e, game_events);

            if (game_events.fire)
            {
                player.fire();
            }
        }

        // Update gameplay.
        auto player_copy = player;
        bool collided    = false;

        // Collision detection loop.
        //
        // TODO: different strategies? if collision first attempt then go smaller than dt_step?
        // Could also try a binary search like thing.
        // TODO: this doesn't handle colliding with several objects at once.
        {
            for (int loop_idx = 0;
                 (loop_idx < 4) && !collided;
                 ++loop_idx)
            {
                player.e.update(dt_step, game_events.player_movement);

                collision::detect_hard_collisions(dt, dt_step, loop_idx, game_events, player, walls, hard_boundaries, collided);
                collision::detect_soft_collisions(player, game_entities, soft_boundaries);
            }

            player.update();
            entity::update(player.crosshair, player.e, game_events.player_rotation, dt_step);
            update_bullets(player.bullets, screen_rect, dt);

            // entity::Player status, detect game over events.
            if (player.hunger < 0.f)
            {
                printf("entity::Player starved.\n");
                player.hunger = 0.5;
                // TODO: Game over event.
            }
        }

        SDL_Rect player_texture_src_rect;
        // Animations
        {
            linalg::Vectorf<2> vel{{player.e.X[1][0], player.e.X[1][1]}};
            player_texture_src_rect = animation::animate(player_texture_descriptor, vel, direction, accumilator, frame);
        }

        // Render player.
        {
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);

            SDL_FRect dst = to_screen_rect(sdl_rect(player.e));
            SDL_Rect  src{0, 0, (int)player.e.w, (int)player.e.h};

            SDL_RenderCopyF(renderer,
                            player.texture,
                            &player_texture_src_rect,
                            &dst);
        }

        // Render crosshair.
        {
            SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);

            // auto const& crosshair = player.crosshair;

            // SDL_FRect dst = to_screen_rect(sdl_rect(crosshair.e));
            // SDL_Rect  src{0, 0, (int)crosshair.e.w, (int)crosshair.e.h};

            // SDL_RenderCopyF(renderer,
            //                 player.texture,
            //                 &player_texture_src_rect,
            //                 &dst);

            auto const& entity = player.crosshair.e;
            SDL_FRect   fdst   = to_screen_rect(sdl_rect(entity));
            SDL_RenderFillRectF(renderer, &fdst);
        }

        // Render bullets.
        {
            SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);

            // auto const& crosshair = player.crosshair;

            // SDL_FRect dst = to_screen_rect(sdl_rect(crosshair.e));
            // SDL_Rect  src{0, 0, (int)crosshair.e.w, (int)crosshair.e.h};

            // SDL_RenderCopyF(renderer,
            //                 player.texture,
            //                 &player_texture_src_rect,
            //                 &dst);

            for (auto& bullet : player.bullets)
            {
                auto const& entity = bullet.e;
                SDL_FRect   fdst   = to_screen_rect(sdl_rect(entity));
                SDL_RenderFillRectF(renderer, &fdst);
            }
        }

        // Render game entities.
        {
            SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
            for (auto& entity : game_entities)
            {
                if (entity.alive)
                {
                    SDL_FRect fdst = to_screen_rect(entity.r);
                    SDL_RenderFillRectF(renderer, &fdst);
                }
            }

            SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xff);
            for (auto& entity : walls)
            {
                SDL_FRect fdst = to_screen_rect(entity.r);
                SDL_RenderFillRectF(renderer, &fdst);
            }

            if (game_events.draw_minkowski)
            {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);

                for (auto& boundary : hard_boundaries)
                {
                    auto dst = to_screen_rect(boundary);
                    SDL_RenderDrawRectF(renderer, &dst);
                }

                for (auto& boundary : soft_boundaries)
                {
                    auto dst = to_screen_rect(boundary);
                    SDL_RenderDrawRectF(renderer, &dst);
                }
            }

            SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
            if (game_events.draw_vectors)
            {
                drawing::draw_vector(renderer,
                                     player.e.X[0][0],
                                     player.e.X[0][1],
                                     player.e.Y[1][0],
                                     player.e.Y[1][1]);
            }
        }

        // Render game hud.
        {
            game_hud.update(player);
            game_hud.render(renderer, game_hud_texture);
        }

        // Render dev hud. This is a bit wasteful if the hud is not enabled
        // however for not it is good to do so we can keep an eye on CPU usage.
        {
            window_update(window_data);
            window_render(renderer, dev_hud_texture, &editor_window, window_data);
        }

        // Copy textures from kiss to the screen.
        {
            SDL_RenderCopy(renderer,
                           game_hud_texture,
                           &game_hud.window.rect,
                           &game_hud.window.rect);

            if (game_events.hud)
            {
                SDL_RenderCopy(renderer,
                               dev_hud_texture,
                               &editor_window.rect,
                               &editor_window.rect);
            }


            SDL_RenderPresent(renderer);
        }


        game_loop.end();
        game_loop.delay();
    }

    serialisation::save(game_state_path, game_events);

    kiss_clean(&objects);
}
