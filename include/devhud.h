#ifndef DEV_HUD_H
#define DEV_HUD_H

#include "gameevents.h"
#include "kiss_sdl.h"
#include "linalg/matrix.hpp"
#include "recthelper.hpp"
#include <SDL2/SDL.h>
#include <array>
#include <string>

struct DevHud {
    kiss_window                      window = {0};
    std::array<kiss_label, 8>        labels;
    std::array<kiss_selectbutton, 2> select_buttons;

    // In preperation that the positioning of the widgets could be refactored.
    int border;
    int x_offset;
    int y_offset;
    int x_center;
    int y_center;

    // Don't position the widgets when the are created. Have a seperate call that
    // sets the position.
    // void set_pos(int row, int col, WidgetLeft, WidgetRight)
    // {
    // }

    DevHud(SDL_Rect const& kiss_screen, int width, int height)
    {
        SDL_Rect window_rect{0, 0, width, height};
        center_a_in_b(window_rect, kiss_screen);

        kiss_window_new(&window,
                        NULL,
                        1,
                        window_rect.x,
                        window_rect.y,
                        window_rect.w,
                        window_rect.h);
        window.bg      = {0x7f, 0x7f, 0x7f, 0x70};
        window.visible = 1;

        border   = 4;
        x_offset = window.rect.x + border;
        y_offset = window.rect.y + border;

        x_center = window.rect.x + window.rect.w / 2;
        y_center = window.rect.y + window.rect.h / 2;

        // int label_text_width  = strlen(label_text.c_str()) * kiss_textfont.advance;
        int label_text_height = kiss_textfont.fontheight + (1 * kiss_normal.h);
        int row_height        = (label_text_height / 2);

        int index = 0;
        int row   = 0;

        int x, y;

        kiss_label_new(&labels[index++],
                       &window,
                       "Time Taken (ms):",
                       x_offset,
                       y_offset + (row * row_height));

        kiss_label_new(&labels[index++],
                       &window,
                       "",
                       x_center + border,
                       y_offset + (row++ * row_height));

        // set_pos can then caclulate the height needed for the row.
        // set_pos(0, labels[0], labels[1]);

        kiss_label_new(&labels[index++],
                       &window,
                       "FPS:",
                       x_offset,
                       y_offset + (row * row_height));

        kiss_label_new(&labels[index++],
                       &window,
                       "",
                       x_center + border,
                       y_offset + (row++ * row_height));

        kiss_label_new(&labels[index++],
                       &window,
                       "Keyboard Input:",
                       x_offset,
                       y_offset + (row * row_height));

        kiss_label_new(&labels[index++],
                       &window,
                       "",
                       x_center + border,
                       y_offset + (row++ * row_height));

        kiss_label_new(&labels[index++],
                       &window,
                       "Draw Vectors:",
                       x_offset,
                       y_offset + (row * row_height));

        kiss_selectbutton_new(&select_buttons[0],
                              &window,
                              x_center + border,
                              y_offset + (row++ * row_height));

        kiss_label_new(&labels[index++],
                       &window,
                       "Draw Minkowski:",
                       x_offset,
                       y_offset + (row * row_height));

        kiss_selectbutton_new(&select_buttons[1],
                              &window,
                              x_center + border,
                              y_offset + (row++ * row_height));
    }

    void handle_events(SDL_Event* event, int* draw, GameEvents& game_events)
    {
        if (kiss_selectbutton_event(&select_buttons[0], event, draw))
        {
            game_events.draw_vectors = select_buttons[0].selected;
        }
        else if (kiss_selectbutton_event(&select_buttons[1], event, draw))
        {
            game_events.draw_minkowski = select_buttons[1].selected;
        }
    }


    // TODO: Should take the structs not individual parameters.
    void update(int time_taken, float fps, linalg::Matrixf<2, 2>& v)
    {
        auto time_taken_text = std::to_string(time_taken);
        kiss_string_copy(labels[1].text,
                         KISS_MAX_LABEL,
                         time_taken_text.c_str(),
                         NULL);

        auto fps_text = std::to_string(fps);
        kiss_string_copy(labels[3].text,
                         KISS_MAX_LABEL,
                         fps_text.c_str(),
                         NULL);

        auto input_text = std::to_string(v[1][0]) + " " + std::to_string(v[1][1]);
        kiss_string_copy(labels[5].text,
                         KISS_MAX_LABEL,
                         input_text.c_str(),
                         NULL);
    }


    void render(SDL_Renderer* renderer, SDL_Texture* texture)
    {
        SDL_SetRenderTarget(renderer, texture);
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
        SDL_RenderClear(renderer);

        kiss_window_draw(&window, renderer);
        for (auto& label : labels)
        {
            kiss_label_draw(&label, renderer);
        }
        for (auto& button : select_buttons)
        {
            kiss_selectbutton_draw(&button, renderer);
        }

        SDL_SetRenderTarget(renderer, nullptr);
    }
};

#endif