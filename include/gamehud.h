#ifndef GAME_HUD_H
#define GAME_HUD_H

#include "entity/core.hpp"
#include "gameevents.h"
#include "kiss_sdl.h"
#include "linalg/matrix.hpp"
#include <SDL2/SDL.h>
#include <string>

inline auto anchor_a_top_of_b(SDL_Rect* a, SDL_Rect* b, int height)
{
    a->x = 0;
    a->y = 0;
    a->w = b->w;
    a->h = height;
}

struct GameHud {
    kiss_window window = {0};
    // kiss_label        labels[7]      = {0};
    kiss_progressbar hunger_bar = {0};

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

    GameHud()
    {
        SDL_Rect kiss_screen{0,
                             0,
                             kiss_screen_width,
                             kiss_screen_height};
        SDL_Rect window_rect{0};

        anchor_a_top_of_b(&window_rect,
                          &kiss_screen,
                          100);

        kiss_window_new(&window,
                        NULL,
                        0,
                        window_rect.x,
                        window_rect.y,
                        window_rect.w,
                        window_rect.h);
        window.bg      = {0x10, 0x10, 0x10, 0xff};
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

        kiss_progressbar_new(&hunger_bar,
                             &window,
                             x_offset,
                             y_offset + (row * row_height),
                             100);
        hunger_bar.fraction = 0.5;
    }

    void handle_events(SDL_Event* event, int* draw, GameEvents& game_events)
    {
    }

    void update(entity::Player& player)
    {
        hunger_bar.fraction = player.health;
    }


    void render(SDL_Renderer* renderer, SDL_Texture* texture)
    {
        SDL_SetRenderTarget(renderer, texture);
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
        SDL_RenderClear(renderer);

        kiss_window_draw(&window, renderer);
        kiss_progressbar_draw(&hunger_bar, renderer);

        SDL_SetRenderTarget(renderer, nullptr);
    }
};


#endif
