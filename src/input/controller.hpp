#pragma once
#include "easing/core.hpp"
#include "gameevents.h"
#include "linalg/matrix.hpp"
#include "typedefs.h"
#include <SDL2/SDL.h>

struct GameState {
    bool quit = false;
};

// TODO: switch should be called button.
struct PlayerActions {
    PlayerActions(easing::Easer& easer)
        : fire(easing::make_debounce_switch(easer, 200))
    {
    }

    linalg::Matrixf<2, 2> player_movement{0};
    linalg::Matrixf<2, 1> player_rotation{0};
    easing::Debounce      fire;
};

struct Controller
{

};

// Useful as we may want to store for live loop editing.
struct Keyboard {
    static constexpr std::array<uint8, 16> scancodes{
        SDL_SCANCODE_LEFT,
        SDL_SCANCODE_RIGHT,
        SDL_SCANCODE_UP,
        SDL_SCANCODE_DOWN, // 3
        SDL_SCANCODE_D,
        SDL_SCANCODE_A,
        SDL_SCANCODE_F,
        SDL_SCANCODE_0, // Unused, 7
        SDL_SCANCODE_0, // Unused, Extras from here
        SDL_SCANCODE_0, // Unused
        SDL_SCANCODE_0, // Unused
        SDL_SCANCODE_0, // Unused, 11
        SDL_SCANCODE_0, // Unused
        SDL_SCANCODE_0, // Unused
        SDL_SCANCODE_H,
        SDL_SCANCODE_ESCAPE};
    std::array<int8, 16> states;
};

inline void update(Controller& k)
{
}

inline void update(Keyboard& k)
{
    uint8 const* keyboard_state = SDL_GetKeyboardState(nullptr);

    for (std::size_t i = 0; i < k.scancodes.size(); ++i)
    {
        k.states[i] = static_cast<int8>(keyboard_state[k.scancodes[i]]);
    }
}

inline void player_update_actions(PlayerActions& actions, Controller& c) {}

inline void player_actions_update(PlayerActions& actions, Keyboard& k)
{
    int8 l = k.states[0];
    int8 r = k.states[1];
    int8 u = k.states[2];
    int8 d = k.states[3];

    int8 cw = k.states[4]; // aim clockwise
    int8 cc = k.states[5]; // aim anticlockwise
    int8 fire = k.states[6];

    {
        int8  x_input = r - l;
        int8  y_input = u - d;
        float x_axis  = static_cast<float>(x_input);
        float y_axis  = static_cast<float>(y_input);

        if (x_input && y_input)
        {
            float norm = std::sqrt((x_input * x_input) + (y_input * y_input));
            x_axis /= norm;
            y_axis /= norm;
        }

        actions.player_movement = {
            {{0.f, 0.f},
             {x_axis, y_axis}}};

        actions.player_rotation = {{0.f, float(cw - cc)}};

        actions.fire.set(fire);
    }
}

inline void game_state_update(GameState& state, Keyboard& k)
{
    state.quit = k.states[15];
}

inline void dev_options_update(DevOptions& dev_opts, Keyboard& k)
{
    int8 h = k.states[14];

    if (h)
    {
        dev_opts.display_hud = !dev_opts.display_hud;
    }
}
