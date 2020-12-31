#include "animation/core.hpp"
#include "collision/core.hpp"
#include "containers/backfill_vector.hpp"
#include "drawing/core.hpp"
#include "easing/core.hpp"
#include "entity/core.hpp"
#include "entity/entityallocator.hpp"
#include "entity/player.hpp"
#include "gameevents.h"
#include "gamehud.h"
#include "input/controller.hpp"
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
extern "C" {
#include "kiss_sdl.h"
}

#include <SDL2/SDL.h>
#include <algorithm>
#include <chrono>
#include <complex>
#include <filesystem>
#include <functional>
#include <iostream>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

// #define DISABLE_SIM
// #define DISABLE_RENDER

//////////////////////////////////////////////////////////////////////////////

using high_res_clock = std::chrono::high_resolution_clock;

//////////////////////////////////////////////////////////////////////////////

namespace serialisation {
extern auto save(std::filesystem::path const&, DevOptions&) -> void;
extern auto load(std::filesystem::path const&, DevOptions&) -> void;
}

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

void make_hard_boundaries(float width, float height, std::vector<entity::EntityStatic>& game_entities, std::vector<SDL_FRect>& hard_boundaries)
{
    linalg::Vectorf<2> origin{{-width, -height}};
    for (auto& hard_entity : game_entities)
    {
        hard_boundaries.push_back(collision::minkowski_boundary(hard_entity, origin));
    }
}

void make_soft_boundaries(float w, float h, std::vector<entity::EntityStatic> const& game_entities, std::vector<SDL_FRect>& soft_boundaries)
{
    linalg::Vectorf<2> origin{{-w, -h}};
    for (auto& soft_entity : game_entities)
    {
        soft_boundaries.push_back(collision::minkowski_boundary(soft_entity, origin));
    }
}

auto make_respawn_points(float w, float h, std::vector<SDL_FRect> const& hard_boundaries)
{
    auto xseg = w / 10.f;
    auto yseg = h / 10.f;

    std::vector<linalg::Vectorf<2>> valid_spawn_points;

    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            linalg::Vectorf<2> point{{xseg * j, yseg * i}};
            for (auto const& hard_entity : hard_boundaries)
            {
                auto collided = collision::is_point_in_rect(point, hard_entity);
                if (!collided)
                {
                    valid_spawn_points.push_back(point);
                }
            }
        }
    }

    assert(valid_spawn_points.size() > 10);
    return valid_spawn_points;
}

auto make_level(float                              w,
                float                              h,
                std::vector<entity::EntityStatic>& soft_entities,
                std::vector<entity::EntityStatic>& walls,
                std::vector<SDL_FRect>&            soft_boundaries,
                std::vector<SDL_FRect>&            hard_boundaries,
                std::vector<SDL_FRect>&            hard_bullet_boundaries,
                std::vector<linalg::Vectorf<2>>&   respawn_points)
{
    soft_entities.push_back(entity::make_food());
    walls.push_back(entity::make_wall());
    {
        make_hard_boundaries(entity::PLAYER_WIDTH,
                             entity::PLAYER_HEIGHT,
                             walls,
                             hard_boundaries);
        make_soft_boundaries(entity::PLAYER_WIDTH,
                             entity::PLAYER_HEIGHT,
                             soft_entities,
                             soft_boundaries);
    }

    {
        make_hard_boundaries(entity::BULLET_WIDTH, entity::BULLET_HEIGHT, walls, hard_bullet_boundaries);
    }

    assert(soft_entities.size() == soft_boundaries.size());
    assert(walls.size() == hard_boundaries.size());

    respawn_points = make_respawn_points(w, h, hard_boundaries);
}

auto player_respawn(entity::Player&                        player,
                    std::vector<linalg::Vectorf<2>> const& valid_points)
{
    int  index = std::rand() % valid_points.size();
    auto point = valid_points.at(index);
    player.respawn(point);
}


auto load_persistent_data(int         argc,
                          char*       argv[],
                          DevOptions& opts)
{

    std::string           argv_str(argv[0]);
    std::filesystem::path exe_base_dir(argv_str.substr(0, argv_str.find_last_of("/")));
    std::filesystem::path game_state_path = (exe_base_dir / "game_state.msgpack");

    if (!std::filesystem::exists(game_state_path))
    {
        printf("%s does not exist.", game_state_path.c_str());
        return game_state_path;
    }

    serialisation::load(game_state_path, opts);
    return game_state_path;
}

int main(int argc, char* argv[])
{
    // Initialise SDL and core components.
    // Other things will break if these aren't done first.
    // -----------------------------------------------------------------------
    SDL_Renderer* renderer;
    SDL_Event     e;
    kiss_array    objects;

    // TODO: This leaks memory.
    kiss_array_new(&objects);
    renderer = kiss_init("Hello kiss_sdl",
                         &objects,
                         SCREEN_WIDTH,
                         SCREEN_HEIGHT);

    // TODO: Why do we need screen_rect?
    SDL_Rect screen_rect{0,
                         0,
                         kiss_screen_width,
                         kiss_screen_height};

    // Persistent Data
    // -----------------------------------------------------------------------
    DevOptions dev_opts;
    auto       game_state_path = load_persistent_data(argc, argv, dev_opts);

    // Engine Data
    // -----------------------------------------------------------------------
    easing::Easer         easer;
    entity::Allocator     alloca;
    // TODO: Below not engine data. Refactor.
    GameState             game_events;
    GameHud               game_hud;

    // Input Setup.
    // -----------------------------------------------------------------------
    KeyboardState         keyboard;
    ControllerState       controller_state;
    PlayerActions         player_actions(easer);
    vector<SDL_GameController*> controllers;

    bool controllers_okay = true;
    enumerate_controllers(controllers, controllers_okay);
    assert(controllers_okay);

    std::vector<entity::Player> players{
        entity::make_player(alloca, {100.f, 100.f, entity::PLAYER_WIDTH, entity::PLAYER_HEIGHT}),
        entity::make_player(alloca, {200.f, 100.f, entity::PLAYER_WIDTH, entity::PLAYER_HEIGHT})};
    auto& player_1 = players[0];
    auto& player_2 = players[1];

    // TODO VariadicDataEditor: Refactor how the window, grid and data are all created.
    VariadicDataEditor window_data(
        // std::tuple{"FPS", (const float*)&fps},
        std::tuple{"Draw Minkowski", &dev_opts.draw_minkowski},
        std::tuple{"Show Vectors", &dev_opts.draw_vectors},
        std::tuple{"Player x", (const float*)&player_1.s->X[0][0]},
        std::tuple{"Player y", (const float*)&player_1.s->X[0][1]});

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

    player_1.texture = load_texture(renderer,
                                    "/home/dwalker0044/Projects/Untitled2D/res/lruptest.png");

    player_2.texture = load_texture(renderer,
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

    // Create game entities and minkowski boundaries.
    std::vector<entity::EntityStatic> soft_entities;
    std::vector<entity::EntityStatic> walls;
    std::vector<SDL_FRect>            soft_boundaries;
    std::vector<SDL_FRect>            hard_boundaries;
    std::vector<SDL_FRect>            hard_bullet_boundaries;
    std::vector<linalg::Vectorf<2>>   respawn_points;
    make_level(kiss_screen_width,
               kiss_screen_height,
               soft_entities,
               walls,
               soft_boundaries,
               hard_boundaries,
               hard_bullet_boundaries,
               respawn_points);


    auto player_texture_descriptor = animation::make_LRUPDescriptor<2>(player_1.texture);
    int  accumilator               = 0;
    int  frame                     = 0;

    animation::Direction direction = animation::Direction::RIGHT;

    // Game loop timer stuff.
    auto         clock = high_res_clock();
    float        fps;
    auto         current_time       = clock.now();
    auto         new_time           = clock.now();
    auto         frame_time         = std::chrono::duration<double>(new_time - current_time);
    double       accumulator        = 0;
    double       render_accumulator = 0;
    double       dit                = 0;
    const double SIM_DT             = 0.05;
    const double SIM_DT_STEP        = SIM_DT / 4;


    while (!game_events.quit)
    {
        {
            new_time   = clock.now();
            frame_time = std::chrono::duration(new_time - current_time);
            dit        = frame_time.count();

            assert(dit < 0.25);

            current_time = new_time;
            accumulator += dit;
            render_accumulator += dit;

            //std::cout << "dt: " << dit;
        }

        // BOOKMARK: Input handling
        update(keyboard);
        if (controllers.size() > 0)
        {
            update(controller_state, controllers[0]);
            for (auto state : controller_state.axis_states)
            {
                printf("%d\t", state);
            }
            printf("\n");

            printf("abxybgslrlrudlri\n");

            for (auto state : controller_state.button_states)
            {
                printf("%1d", state);
            }
            printf("\n");
        }
        player_actions_update(player_actions, controller_state);
        game_state_update(game_events, keyboard);
        dev_options_update(dev_opts, keyboard);

        // handle_input_states(e, game_events, dev_opts);

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                game_events.quit = 1;
            }
            // TODO: Not really sure what draw does.
            int draw = 0;
            editor_handle_events(window_data, &e, &draw);
            // game_hud.handle_events(&e, &draw, game_events);
            // handle_input_event(e, game_events, dev_opts);
        }

        if (player_actions.fire.get())
        {
            player_1.fire();
        }

#ifdef DISABLE_SIM
#else
        bool simulation_ran = false;
        while (accumulator > SIM_DT)
        {
            //printf("s");
            simulation_ran = true;
            accumulator -= SIM_DT;

            // Update gameplay.
            auto player_copy = player_1;
            bool collided    = false;

            // Collision detection loop.
            //
            // TODO: different strategies? if collision first attempt then go smaller than dt_step?
            // Could also try a binary search like thing.
            // TODO: this doesn't handle colliding with several objects at once.

            for (int loop_idx = 0;
                 (loop_idx < 4) && !collided;
                 ++loop_idx)
            {
                entity::set_input(player_1.s,
                                  player_actions.player_movement);
                entity::integrate(player_1.s,
                                  SIM_DT_STEP);

                collision::detect_hard_collisions(SIM_DT,
                                                  SIM_DT_STEP,
                                                  loop_idx,
                                                  player_actions,
                                                  player_1,
                                                  walls,
                                                  hard_boundaries,
                                                  collided);

                // Note(DW): Doesn't need dt as player position is updated and soft collisions are static.
                collision::detect_soft_collisions(player_1,
                                                  soft_entities,
                                                  soft_boundaries);
            }

            entity::set_input(player_1.aim.s,
                              player_actions.player_rotation);
            entity::integrate(player_1.aim.s,
                              SIM_DT);

            //entity::integrate(player_1.crosshair,
            //SIM_DT);

            update_bullets(player_1,
                           players,
                           hard_bullet_boundaries,
                           screen_rect,
                           SIM_DT);

            for (auto& player : players)
            {
                if (player.health < 0.f)
                {
                    player_respawn(player, respawn_points);
                }
            }
        } // end sim loop

#endif // DISABLE_SIM

        if (simulation_ran)
        {
            // printf(".");
            update(alloca);
        }

        // Perform forward integration.
        // This uses the velocities from the last simulation step to estimate the
        // location of the entities for the current render step.
        // This improves how the objects move across the screen as the positions
        // are much better aligned to what they should be for the render step.
        // Without this, we'd get large jumps, as there is always some time
        // remaining after the simulation.
        interpolate(alloca, dit);

        // easing
        easer.step(dit * 1000);

        bool render_ran = false;
        while (render_accumulator > 0.03)
        {
            //printf("r");
            render_accumulator -= 0.03;

            // Want to check that the render only runs once per loop.
            assert(render_ran == false);
            render_ran = true;

#ifdef DISABLE_RENDER
#else

            SDL_Rect player_texture_src_rect;
            // Animations
            {
                auto const& pX = player_1.s->X;

                linalg::Vectorf<2> vel{{pX[1][0], pX[1][1]}};
                player_texture_src_rect = animation::animate(player_texture_descriptor,
                                                             vel,
                                                             direction,
                                                             accumilator,
                                                             frame);
            }

            // Render player.
            {
                SDL_SetRenderTarget(renderer, nullptr);
                SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderClear(renderer);

                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);

                for (auto const& player : players)
                {
                    SDL_FRect dst = to_screen_rect(sdl_rect(player.r));

                    SDL_RenderCopyF(renderer,
                                    player.texture,
                                    &player_texture_src_rect,
                                    &dst);
                }

                // Render crosshair.
                {
                    entity::update_crosshair(player_1);

                    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);

                    SDL_FRect fdst = to_screen_rect(player_1.crosshair.rect);
                    SDL_RenderFillRectF(renderer, &fdst);
                }

                // Render bullets.
                {
                    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);

                    for (auto& bullet : player_1.bullets)
                    {
                        SDL_FRect fdst = to_screen_rect(sdl_rect(bullet.r));
                        SDL_RenderFillRectF(renderer, &fdst);
                    }
                }
            }

            // Render game entities.
            {
                SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
                for (auto& entity : soft_entities)
                {
                    if (entity.alive)
                    {
                        SDL_FRect fdst = to_screen_rect(entity.rect);
                        SDL_RenderFillRectF(renderer, &fdst);
                    }
                }

                SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xff);
                for (auto& entity : walls)
                {
                    SDL_FRect fdst = to_screen_rect(entity.rect);
                    SDL_RenderFillRectF(renderer, &fdst);
                }

                if (dev_opts.draw_minkowski)
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
                if (dev_opts.draw_vectors)
                {
                    auto const& pX = player_1.r->X;
                    auto const& pY = player_1.r->Y;
                    drawing::draw_vector(renderer,
                                         pX[0][0],
                                         pX[0][1],
                                         pY[1][0],
                                         pY[1][1]);
                }
            }

#endif // DISABLE_RENDER

            // Render game hud.
            {
                game_hud.update(player_2);
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

                if (dev_opts.display_hud)
                {
                    SDL_RenderCopy(renderer,
                                   dev_hud_texture,
                                   &editor_window.rect,
                                   &editor_window.rect);
                }


                SDL_RenderPresent(renderer);
            }
        }

        SDL_Delay(1);
    } // end while

    serialisation::save(game_state_path, dev_opts);
    free_controllers(controllers);
    kiss_clean(&objects);
}
