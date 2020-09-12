#ifndef DEV_HUD_H
#define DEV_HUD_H

#include "gameevents.h"
#include "kiss_sdl.h"
#include "linalg/matrix.hpp"
#include <SDL2/SDL.h>
#include <string>

auto center_a_in_b(SDL_Rect* a, SDL_Rect* b)
{
    int x = (b->w - a->w) / 2;
    int y = (b->h - a->h) / 2;
    a->x  = x;
    a->y  = y;
}

struct DevHud {
    kiss_window       window         = {0};
    kiss_label        labels[7]      = {0};
    kiss_selectbutton enable_vectors = {0};

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

    DevHud()
    {
        SDL_Rect kiss_screen{0,
                             0,
                             kiss_screen_width,
                             kiss_screen_height};
        SDL_Rect window_rect{0,
                             0,
                             kiss_screen_width / 2,
                             kiss_screen_height / 2};

        center_a_in_b(&window_rect,
                      &kiss_screen);

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

        kiss_selectbutton_new(&enable_vectors,
                              &window,
                              x_center + border,
                              y_offset + (row++ * row_height));

        // kiss_button_new(&button,
        //                 &window,
        //                 "OK",
        //                 window.rect.x + (x_center - (kiss_normal.w / 2)),
        //                 labels[2].rect.y + label_text_height);
    }

    void handle_events(SDL_Event* event, int* draw, GameEvents& game_events)
    {
        if (kiss_selectbutton_event(&enable_vectors, event, draw))
        {
            game_events.draw_vectors = enable_vectors.selected;
            printf("draw vs: %d\n", game_events.draw_vectors);
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
        kiss_label_draw(&labels[0], renderer);
        kiss_label_draw(&labels[1], renderer);
        kiss_label_draw(&labels[2], renderer);
        kiss_label_draw(&labels[3], renderer);
        kiss_label_draw(&labels[4], renderer);
        kiss_label_draw(&labels[5], renderer);
        kiss_label_draw(&labels[6], renderer);
        kiss_selectbutton_draw(&enable_vectors, renderer);
        // kiss_button_draw(&button, renderer);

        SDL_SetRenderTarget(renderer, nullptr);
    }
};

#endif