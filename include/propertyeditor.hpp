#ifndef PROPERTY_EDITOR_HPP
#define PROPERTY_EDITOR_HPP

#include "kiss_sdl.h"
#include <SDL2/SDL.h>
#include <string>
#include <vector>


struct text_entry {
    kiss_entry k;
    char       data[KISS_MAX_LENGTH];
};

struct float_entry {
    kiss_entry k;
    char       data[KISS_MAX_LENGTH];
};

struct int_entry {
    kiss_entry k;
    char       data[KISS_MAX_LENGTH];
};

enum class WidgetType {
    number,
    integer,
    binary,
    text,
    display
};

struct WindowData {
    std::vector<float*> floats;
    std::vector<int*>   integers;
    std::vector<bool*>  bools;
    std::vector<char*>  strings;
    // std::vector<std::string>  fmt_buffer;

    std::vector<kiss_label>        labels;
    std::vector<kiss_label>        widget_labels;
    std::vector<kiss_selectbutton> select_buttons;
    // std::vector<kiss_entry>        entry_boxes;
    std::vector<text_entry>  text_entry_boxes;
    std::vector<float_entry> float_entry_boxes;
    std::vector<int_entry>   int_entry_boxes;

    struct WidgetDataMap {
        WidgetType  type;
        char const* label;
        size_t      data_id;
        size_t      widget_id;
    };

    std::vector<WidgetDataMap> sequence;
};

auto make_window_from_values(kiss_window&, SDL_Rect&, int, int, WindowData&) -> void;

void window_render(SDL_Renderer*, SDL_Texture*, WindowData&, kiss_window&);
auto window_handle_events(SDL_Event* event, int* draw, WindowData& data) -> void;
#endif // PROPERTY_EDITOR_HPP