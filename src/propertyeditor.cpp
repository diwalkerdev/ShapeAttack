#include "propertyeditor.hpp"
#include "fmt/core.h"
#include "gameevents.h"
#include "recthelper.hpp"

auto window_data_add_concrete_tp(WindowData& window, char const* label, float* data)
{
    window.labels.push_back({});
    window.float_entry_boxes.push_back({});
    window.floats.push_back(data);
    // window.fmt_buffer.push_back({});

    window.sequence.push_back({WidgetType::number,
                               label,
                               window.floats.size() - 1,
                               window.float_entry_boxes.size() - 1});
}

auto window_data_add_concrete_tp(WindowData& window, char const* label, int* data)
{
    window.labels.push_back({});
    window.int_entry_boxes.push_back({});
    window.integers.push_back(data);
    // window.fmt_buffer.push_back({});

    window.sequence.push_back({WidgetType::number,
                               label,
                               window.floats.size() - 1,
                               window.int_entry_boxes.size() - 1});
}


auto window_data_add_concrete_tp(WindowData& window, char const* label, char* data)
{
    window.labels.push_back({});
    window.strings.push_back(data);
    window.text_entry_boxes.push_back({});

    window.sequence.push_back({WidgetType::text,
                               label,
                               window.strings.size() - 1,
                               window.text_entry_boxes.size() - 1});
}

auto window_data_add_concrete_tp(WindowData& window, char const* label, bool* data)
{
    window.labels.push_back({});
    window.bools.push_back(data);
    window.select_buttons.push_back({});
    window.sequence.push_back({WidgetType::binary,
                               label,
                               window.bools.size() - 1,
                               window.select_buttons.size() - 1});
}


template <typename Tp>
auto window_data_add(WindowData& window_data, std::tuple<char const*, Tp*> data)
{
    window_data_add_concrete_tp(window_data, std::get<0>(data), std::get<1>(data));
}

template <typename Tp, typename... Args>
auto window_data_add(WindowData& window_data, std::tuple<char const*, Tp*> data, Args... args)
{
    window_data_add_concrete_tp(window_data, std::get<0>(data), std::get<1>(data));
    window_data_add(window_data, args...);
}

template <typename... Args>
auto make_window(WindowData& window_data, Args... args)
{
    window_data_add(window_data, args...);
}


void window_init(kiss_window& window, WindowData& data)
{
    auto border   = 4;
    auto x_offset = window.rect.x + border;
    auto y_offset = window.rect.y + border;
    auto x_center = window.rect.x + window.rect.w / 2;
    auto y_center = window.rect.y + window.rect.h / 2;

    int row = 0;

    int label_text_height = kiss_textfont.fontheight + (1 * kiss_normal.h);
    int row_height        = (label_text_height / 2) + 10;

    int errors;
    // Init the widgets.
    for (int index = 0; index < data.sequence.size(); ++index)
    {
        auto& element = data.sequence[index];

        WidgetType  type      = element.type;
        char const* label     = element.label;
        size_t      data_id   = element.data_id;
        size_t      widget_id = element.widget_id;

        kiss_label_new(&data.labels[index],
                       &window,
                       label,
                       x_offset,
                       y_offset + (row * row_height));

        if (type == WidgetType::number)
        {
            float_entry* entry  = &data.float_entry_boxes[widget_id];
            float*       number = data.floats[data_id];

            auto as_text = fmt::format("{0}", *number);
            strncpy(&entry->data[0], as_text.c_str(), KISS_MAX_LENGTH);

            errors = kiss_entry_new(&entry->k,
                                    &window,
                                    false, // decorate
                                    &entry->data[0],
                                    x_center + border,
                                    y_offset + (row++ * row_height),
                                    100);
            assert(errors == 0);
        }
        else if (type == WidgetType::integer)
        {
            int_entry* entry  = &data.int_entry_boxes[widget_id];
            int*       number = data.integers[data_id];

            auto as_text = fmt::format("{0}", *number);
            strncpy(&entry->data[0], as_text.c_str(), KISS_MAX_LENGTH);

            errors = kiss_entry_new(&entry->k,
                                    &window,
                                    false, // decorate
                                    &entry->data[0],
                                    x_center + border,
                                    y_offset + (row++ * row_height),
                                    100);
            assert(errors == 0);
        }
        else if (type == WidgetType::text)
        {
            text_entry* entry = &data.text_entry_boxes[widget_id];
            char*       text  = data.strings[data_id];

            strncpy(&entry->data[0], text, KISS_MAX_LENGTH);

            errors = kiss_entry_new(&entry->k,
                                    &window,
                                    false, // decorate
                                    &entry->data[0],
                                    x_center + border,
                                    y_offset + (row++ * row_height),
                                    100);
            assert(errors == 0);
        }
        else if (type == WidgetType::binary)
        {
            kiss_selectbutton_new(&data.select_buttons[widget_id],
                                  &window,
                                  x_center + border,
                                  y_offset + (row++ * row_height));
        }
        else if (type == WidgetType::display)
        {
        }
        else
        {
            assert(false);
        }
    }
}

void window_render(SDL_Renderer* renderer, SDL_Texture* texture, WindowData& data, kiss_window& window)
{
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
    SDL_RenderClear(renderer);

    kiss_window_draw(&window, renderer);

    for (auto& label : data.labels)
    {
        kiss_label_draw(&label, renderer);
    }

    for (auto& entry : data.text_entry_boxes)
    {
        kiss_entry_draw(&entry.k, renderer);
    }

    for (auto& entry : data.float_entry_boxes)
    {
        kiss_entry_draw(&entry.k, renderer);
    }

    for (auto& entry : data.int_entry_boxes)
    {
        kiss_entry_draw(&entry.k, renderer);
    }

    for (auto& button : data.select_buttons)
    {
        kiss_selectbutton_draw(&button, renderer);
    }

    SDL_SetRenderTarget(renderer, nullptr);
}


auto make_window_from_values(kiss_window& window, SDL_Rect& kiss_screen, int width, int height, WindowData& data) -> void
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

    float num                   = M_PI;
    bool  x                     = true;
    char  name[KISS_MAX_LENGTH] = {0};
    strcpy(&name[0], "David");

    make_window(data,
                std::tuple{"Number", &num},
                std::tuple{"X", &x},
                std::tuple{"Name", &name[0]});

    fmt::print("floats: {0}\n", data.floats.size());
    fmt::print("ints: {0}\n", data.bools.size());
    fmt::print("strings: {0}\n", data.strings.size());
    for (auto& element : data.sequence)
    {
        fmt::print("{0} {1} {2}\n", element.type, element.data_id, element.widget_id);
    }

    window_init(window, data);
}


// void handle_events(SDL_Event* event, int* draw, GameEvents& game_events)
// {
//     if (kiss_selectbutton_event(&select_buttons[0], event, draw))
//     {
//         game_events.draw_vectors = select_buttons[0].selected;
//     }
//     else if (kiss_selectbutton_event(&select_buttons[1], event, draw))
//     {
//         game_events.draw_minkowski = select_buttons[1].selected;
//     }
// }


auto window_handle_events(SDL_Event* event, int* draw, WindowData& data) -> void
{
    for (auto& element : data.sequence)
    {
        WidgetType type      = element.type;
        size_t     data_id   = element.data_id;
        size_t     widget_id = element.widget_id;

        if (type == WidgetType::number)
        {
            float_entry* entry = &data.float_entry_boxes[widget_id];

            if (kiss_entry_event(&entry->k, event, draw))
            {
                float* number = data.floats[data_id];
                char*  text   = &entry->k.text[0];

                // TODO: Validation.
                printf("Validating float\n");
                *number = std::stof(text);
            }
        }
        else if (type == WidgetType::integer)
        {
            int_entry* entry = &data.int_entry_boxes[widget_id];

            if (kiss_entry_event(&entry->k, event, draw))
            {
                int*  number = data.integers[data_id];
                char* text   = &entry->k.text[0];

                // TODO: Validation.
                printf("Validating int\n");
                *number = std::stof(text);
            }
        }
        else if (type == WidgetType::text)
        {
            text_entry* entry = &data.text_entry_boxes[widget_id];

            if (kiss_entry_event(&entry->k, event, draw))
            {
                char* text = data.strings[data_id];
                strncpy(text, &entry->k.text[0], KISS_MAX_LENGTH);

                printf("Validating text\n");
            }
        }
        else if (type == WidgetType::binary)
        {
            kiss_selectbutton* select = &data.select_buttons[widget_id];
            bool*              choice = data.bools[data_id];

            if (kiss_selectbutton_event(select, event, draw))
            {
                *choice = select->selected;
            }
        }
        else if (type == WidgetType::display)
        {
        }
        else
        {
            assert(false);
        }
    }
}