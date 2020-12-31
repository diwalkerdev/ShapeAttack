#pragma once
#include "easing/core.hpp"
#include "gameevents.h"
#include "linalg/matrix.hpp"
#include "typedefs.h"
#include <SDL2/SDL.h>

constexpr int MAX_NUM_CONTROLLERS{4};

// TODO GameState should be moved somewhere else.
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

// Controller Stuff
// ---------------------------------------------------------------------------
struct ControllerState {
    array<int16, 6> axis_states;
    array<int8, 17> button_states;
};

inline void enumerate_controllers(vector<SDL_GameController*>& controllers, bool& is_okay)
{
    auto num_joysticks = SDL_NumJoysticks();
    if (num_joysticks < 1)
    {
        printf("No controllers connected\n");
        return;
    }

    // Only allow a maximum of 4 controllers to be connected.
    num_joysticks = std::min(num_joysticks, MAX_NUM_CONTROLLERS);

    for (size_t i = 0; i < num_joysticks; ++i)
    {
        auto* joytick = SDL_JoystickOpen(i);
        if (!SDL_IsGameController(i))
        {
            printf("Warning: Controller is not a gamepad, only gamepads are suppored.\n");
            continue;
        }

        auto* controller = SDL_GameControllerOpen(i);
        if (!controller)
        {
            printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
            is_okay = false;
            continue;
        }

        controllers.push_back(controller);
    }
}

inline void free_controllers(vector<SDL_GameController*> controllers)
{
    for (auto* c : controllers)
    {
        SDL_GameControllerClose(c);
    }
}


inline void update(ControllerState& k, SDL_GameController* joystick)
{
    // NOTE: SDL_CONTROLLER_AXIS_INVALID is -1.
    // NOTE: SDL_CONTROLLER_BUTTON_INVALID is -1.
    {
        k.axis_states[SDL_CONTROLLER_AXIS_MAX - 1] = SDL_GameControllerGetAxis(
            joystick,
            SDL_CONTROLLER_AXIS_INVALID);
        int begin = static_cast<int>(SDL_CONTROLLER_AXIS_LEFTX);
        int end   = static_cast<int>(SDL_CONTROLLER_AXIS_MAX);
        for (auto i = begin; i < end; ++i)
        {
            k.axis_states[i] = SDL_GameControllerGetAxis(
                joystick,
                static_cast<SDL_GameControllerAxis>(i));
        }
    }

    {
        k.button_states[SDL_CONTROLLER_BUTTON_MAX - 1] = SDL_GameControllerGetButton(
            joystick,
            SDL_CONTROLLER_BUTTON_INVALID);
        int begin = static_cast<int>(SDL_CONTROLLER_BUTTON_A);
        int end   = static_cast<int>(SDL_CONTROLLER_BUTTON_MAX);
        for (int i = begin; i < end; ++i)
        {
            k.button_states[i] = SDL_GameControllerGetButton(
                joystick,
                static_cast<SDL_GameControllerButton>(i));
        }
    }
}

inline void player_actions_update(PlayerActions& actions, ControllerState& k)
{
    int8 u = k.button_states[SDL_CONTROLLER_BUTTON_DPAD_UP];
    int8 d = k.button_states[SDL_CONTROLLER_BUTTON_DPAD_DOWN];
    int8 l = k.button_states[SDL_CONTROLLER_BUTTON_DPAD_LEFT];
    int8 r = k.button_states[SDL_CONTROLLER_BUTTON_DPAD_RIGHT];
    //
    // aim clockwise and counter clockwise respectively
    int8 cw   = k.button_states[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER];
    int8 cc   = k.button_states[SDL_CONTROLLER_BUTTON_LEFTSHOULDER];
    int8 fire = k.button_states[SDL_CONTROLLER_BUTTON_B];

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

// Keyboard Stuff
// ---------------------------------------------------------------------------

// Useful as we may want to store for live loop editing.
struct KeyboardState {
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

inline void update(KeyboardState& k)
{
    uint8 const* keyboard_state = SDL_GetKeyboardState(nullptr);

    for (std::size_t i = 0; i < k.scancodes.size(); ++i)
    {
        k.states[i] = static_cast<int8>(keyboard_state[k.scancodes[i]]);
    }
}

inline void player_actions_update(PlayerActions& actions, KeyboardState& k)
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

inline void game_state_update(GameState& state, KeyboardState& k)
{
    state.quit = k.states[15];
}

inline void dev_options_update(DevOptions& dev_opts, KeyboardState& k)
{
    int8 h = k.states[14];

    if (h)
    {
        dev_opts.display_hud = !dev_opts.display_hud;
    }
}
