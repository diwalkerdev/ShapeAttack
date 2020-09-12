#include "kiss_sdl.h"
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
    kiss_window window    = {0};
    kiss_label  labels[4] = {0};
    // kiss_button button    = {0};

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
                        0,
                        window_rect.x,
                        window_rect.y,
                        window_rect.w,
                        window_rect.h);
        window.bg      = {0x7f, 0x7f, 0x7f, 0x70};
        window.visible = 1;

        int wx_center = window.rect.w / 2;
        int wy_center = window.rect.h / 2;
        // int label_text_width  = strlen(label_text.c_str()) * kiss_textfont.advance;
        int label_text_height = kiss_textfont.fontheight + (1 * kiss_normal.h);
        int row_height        = (label_text_height / 2);

        int row = 0;
        kiss_label_new(&labels[0],
                       &window,
                       "Time Taken (ms):",
                       window.rect.x,
                       window.rect.y + (row * row_height));

        kiss_label_new(&labels[1],
                       &window,
                       "",
                       window.rect.x + 150,
                       window.rect.y + (row++ * row_height));

        kiss_label_new(&labels[2],
                       &window,
                       "FPS:",
                       window.rect.x,
                       window.rect.y + (row * row_height));

        kiss_label_new(&labels[3],
                       &window,
                       "",
                       window.rect.x + 150,
                       window.rect.y + (row++ * row_height));

        // kiss_button_new(&button,
        //                 &window,
        //                 "OK",
        //                 window.rect.x + (wx_center - (kiss_normal.w / 2)),
        //                 labels[2].rect.y + label_text_height);
    }

    void update(int time_taken, float fps)
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
        // kiss_button_draw(&button, renderer);

        SDL_SetRenderTarget(renderer, nullptr);
    }
};