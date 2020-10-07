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

template <typename Tp>
struct readonly {
    kiss_label k;
    Tp const*  data;
    char       tbuf[KISS_MAX_LENGTH];
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
    // std::vector<readonly> readonly_boxes;

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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline auto associated_widget(float*) -> float_entry;
inline auto associated_widget(bool*) -> kiss_selectbutton;
inline auto associated_widget(float const*) -> kiss_label;

template <size_t idx, typename T>
struct GetHelper;


template <typename... T>
struct DataStructure {
};

template <typename T, typename... Rest>
struct DataStructure<const char*, T, Rest...> {
    DataStructure(char const* label, T param, const Rest&... rest)
        : label(label)
        , data(param)
        , rest(rest...)
    {
    }

    char const*                       label;
    kiss_label                        label_widget;
    T                                 data;
    decltype(associated_widget(data)) data_widget;

    DataStructure<Rest...> rest;

    template <size_t idx>
    auto get()
    {
        return GetHelper<idx, DataStructure<char const*, T, Rest...>>::get(*this);
    }
};

// Deduction guide
template <typename Tp, typename... Rest>
DataStructure(char const*, Tp*, Rest...) -> DataStructure<const char*, Tp*, Rest...>;


template <typename T, typename... Rest>
struct GetHelper<0, DataStructure<char const*, T, Rest...>> {
    static DataStructure<char const*, T, Rest...> get(DataStructure<char const*, T, Rest...>& the_struct)
    {
        return the_struct;
    }
};

template <size_t idx, typename T, typename... Rest>
struct GetHelper<idx, DataStructure<char const*, T, Rest...>> {
    static auto get(DataStructure<char const*, T, Rest...>& the_struct)
    {
        return GetHelper<idx - 1, DataStructure<Rest...>>::get(the_struct.rest);
    }
};

///////////////////////////////////////////////////////////////////////////////


#include <cassert>
#include <iostream>

template <typename P = char const*, typename T>
auto print(DataStructure<P, T>& ds)
{
    std::cout << *(ds.data) << std::endl;
}


template <typename P = char const*, typename T, typename... Rest>
auto print(DataStructure<P, T, Rest...>& ds) -> std::enable_if_t<sizeof...(Rest) != 0, void>
{
    std::cout << *(ds.data) << std::endl;
    print(ds.rest);
}

///////////////////////////////////////////////////////////////////////////////

struct Grid2x2 {
    Grid2x2(kiss_window* window)
    {
        auto border   = 4;
        auto x_offset = window->rect.x + border;
        auto y_offset = window->rect.y + border;
        auto x_center = window->rect.x + window->rect.w / 2;
        auto y_center = window->rect.y + window->rect.h / 2;

        int label_text_height = kiss_textfont.fontheight + (1 * kiss_normal.h);
        int row_height        = (label_text_height / 2) + 10;
    }

    void get(int col, int& x, int& y)
    {
        // x = x_center + border;
        // y = y_offset + (row * row_height);
        x = 0;
        // y = (row * row_height);
        y = row * 50;
    }

    int row{0};

    int border;
    int x_offset;
    int y_offset;
    int x_center;
    int y_center;

    int label_text_height;
    int row_height;
};

#include "fmt/core.h"

///////////////////////////////////////////////////////////////////////////////

template <typename... Rest>
void init(kiss_window* window, Grid2x2& grid, DataStructure<const char*, float*, Rest...>& ds)
{
    float_entry* entry  = &ds.data_widget;
    float*       number = ds.data;

    auto as_text = fmt::format("{0}", *number);
    strncpy(&entry->data[0], as_text.c_str(), KISS_MAX_LENGTH);

    int x, y;
    grid.get(0, x, y);

    int errors = kiss_entry_new(&entry->k,
                                window,
                                true, // decorate
                                &entry->data[0],
                                x,
                                y,
                                100);
    assert(errors == 0);
}

template <typename... Rest>
void init(kiss_window* window, Grid2x2& grid, DataStructure<const char*, float const*, Rest...>& ds)
{
    kiss_label*  label  = &ds.data_widget;
    float const* number = ds.data;

    auto as_text = fmt::format("{0}", *number);

    int x, y;
    grid.get(0, x, y);

    int errors = kiss_label_new(label,
                                window,
                                as_text.c_str(),
                                x,
                                y);

    assert(errors == 0);
}

template <typename... Rest>
void init(kiss_window* window, Grid2x2& grid, DataStructure<const char*, bool*, Rest...>& ds)
{
    kiss_selectbutton* select = &ds.data_widget;
    bool*              value  = ds.data;

    int x, y;
    grid.get(0, x, y);

    int errors = kiss_selectbutton_new(select,
                                       window,
                                       x,
                                       y);
    assert(errors == 0);
}

///////////////////////////////////////////////////////////////////////////////

template <typename P = char const*, typename T>
auto window_init(DataStructure<P, T>& ds, kiss_window* window, Grid2x2& grid)
{
    init(window, grid, ds);
}


template <typename P = char const*, typename T, typename... Rest>
auto window_init(DataStructure<P, T, Rest...>& ds, kiss_window* window, Grid2x2& grid) -> std::enable_if_t<sizeof...(Rest) != 0, void>
{
    init(window, grid, ds);
    grid.row += 1;

    window_init(ds.rest, window, grid);
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Rest>
void draw(SDL_Renderer* renderer, DataStructure<const char*, bool*, Rest...>& ds)
{
    kiss_selectbutton_draw(&ds.data_widget, renderer);
}

template <typename... Rest>
void draw(SDL_Renderer* renderer, DataStructure<const char*, float*, Rest...>& ds)
{
    kiss_entry_draw(&ds.data_widget.k, renderer);
}

template <typename... Rest>
void draw(SDL_Renderer* renderer, DataStructure<const char*, float const*, Rest...>& ds)
{
    kiss_label_draw(&ds.data_widget, renderer);
}

template <typename P = char const*, typename T>
auto window_draw(DataStructure<P, T>& ds, SDL_Renderer* renderer) -> void
{
    // SDL_SetRenderTarget(renderer, texture);
    // SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
    // SDL_RenderClear(renderer);

    // kiss_window_draw(&window, renderer);

    draw(renderer, ds);

    // SDL_SetRenderTarget(renderer, nullptr);
}

template <typename P = char const*, typename T, typename... Rest>
auto window_draw(DataStructure<P, T, Rest...>& ds, SDL_Renderer* renderer) -> std::enable_if_t<sizeof...(Rest) != 0, void>
{
    // SDL_SetRenderTarget(renderer, texture);
    // SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
    // SDL_RenderClear(renderer);

    // kiss_window_draw(&window, renderer);

    draw(renderer, ds);
    window_draw(ds.rest, renderer);

    // SDL_SetRenderTarget(renderer, nullptr);
}

///////////////////////////////////////////////////////////////////////////////

inline void test_function(kiss_window* window, SDL_Renderer* renderer, SDL_Texture* texture)
{
    float const pi     = M_PI;
    bool const  truthy = true;
    float       number = 1.234;
    bool        choice = false;

    DataStructure window_data("Number", &number, "PI", &pi, "Choice", &choice);
    // auto          first = window_data.get<0>();
    // std::cout << *first.data << "\n";

    // auto secn = window_data.get<1>();
    // std::cout << *secn.data << "\n";

    // auto thid = window_data.get<2>();
    // std::cout << *thid.data << "\n";
    print(window_data);


    Grid2x2 grid(window);

    window_init(window_data, window, grid);

    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
    SDL_RenderClear(renderer);

    kiss_window_draw(window, renderer);
    window_draw(window_data, renderer);

    SDL_SetRenderTarget(renderer, nullptr);
}

#endif // PROPERTY_EDITOR_HPP