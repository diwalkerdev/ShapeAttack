#ifndef PROPERTY_EDITOR_HPP
#define PROPERTY_EDITOR_HPP

#include "fmt/core.h"
#include "kiss_sdl.h"
#include <SDL2/SDL.h>
#include <cassert>
#include <iostream>
#include <string>
#include <tuple>
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

inline auto associated_widget(float*) -> float_entry;
inline auto associated_widget(bool*) -> kiss_selectbutton;
inline auto associated_widget(float const*) -> kiss_label;
inline auto associated_widget(bool const*) -> kiss_label;

// 1. General variadic template declaration.
//    Declare that the follow struct will be variadic.
template <typename... Tp>
struct DataStructure {
};

// 2. Specialisation - deals with the variadic parameter unpacking.
//    Given vargs, how are they consumed? 1 at a time? 2 at a time?
template <typename Tp, typename... Args>
struct DataStructure<std::tuple<const char*, Tp*>, Args...> {
    const char*                       label;
    Tp*                               data;
    kiss_label                        label_widget;
    decltype(associated_widget(data)) data_widget;

    DataStructure<Args...> args;

    DataStructure(std::tuple<const char*, Tp*> tup, const Args&... args)
        : label(std::get<0>(tup))
        , data(std::get<1>(tup))
        , args(args...)
    {
    }
};

// 3. Deduction Guide.
//    Specifies the specialisation from a constructor call.
//    Without this the compiler will attempt to use the general form with doesn't have a suitable constructor.
template <typename Tp, typename... Args>
DataStructure(std::tuple<const char*, Tp*>, Args...) -> DataStructure<std::tuple<const char*, Tp*>, Args...>;


template <typename Tp, typename... Args>
using GenericWindowArgs = DataStructure<std::tuple<const char*, Tp>, Args...>;


// 4. Get
//    Allows access to the 'elements' of variadic structure.
template <std::size_t index, typename Tp, typename... Args>
inline constexpr auto get(GenericWindowArgs<Tp*, Args...>& ds)
{
    if constexpr (index == 0)
    {
        return ds;
    }
    else
    {
        return get<index - 1>(ds.args);
    }
}


template <typename Tp, typename... Args>
constexpr auto print(GenericWindowArgs<Tp*, Args...>& ds)
{
    std::cout << ds.label << " " << *ds.data << '\n';
    if constexpr (sizeof...(Args) > 0)
    {
        print(ds.args);
    }
}


///////////////////////////////////////////////////////////////////////////////


struct Grid2x2 {
    Grid2x2(kiss_window* window)
    {
        border   = 4;
        x_offset = window->rect.x + border;
        y_offset = window->rect.y + border;
        x_center = window->rect.x + window->rect.w / 2;
        y_center = window->rect.y + window->rect.h / 2;

        label_text_height = kiss_textfont.fontheight + (1 * kiss_normal.h);
        row_height        = 50; // (label_text_height / 2) + 10;
    }

    void get(int col, int& x, int& y) const
    {
        x = (col == 0) ? x_offset + border : x_center + border;
        y = y_offset + (row * row_height);
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


///////////////////////////////////////////////////////////////////////////////

inline void init_widget_label(kiss_window* window, Grid2x2& grid, kiss_label* label_widget, const char* label_text)
{
    int x, y;
    grid.get(0, x, y);

    int errors = kiss_label_new(label_widget,
                                window,
                                label_text,
                                x,
                                y);
}

template <typename... Args>
void init(kiss_window* window, Grid2x2& grid, GenericWindowArgs<float*, Args...>& ds)
{
    float_entry* entry  = &ds.data_widget;
    float*       number = ds.data;

    init_widget_label(window, grid, &ds.label_widget, ds.label);

    auto as_text = fmt::format("{0}", *number);
    strncpy(&entry->data[0], as_text.c_str(), KISS_MAX_LENGTH);

    int x, y;
    grid.get(1, x, y);

    int errors = kiss_entry_new(&entry->k,
                                window,
                                true, // decorate
                                &entry->data[0],
                                x,
                                y,
                                100);
    assert(errors == 0);
}

template <typename... Args>
void init(kiss_window* window, Grid2x2& grid, GenericWindowArgs<float const*, Args...>& ds)
{
    kiss_label*  label  = &ds.data_widget;
    float const* number = ds.data;

    init_widget_label(window, grid, &ds.label_widget, ds.label);

    auto as_text = fmt::format("{0}", *number);

    int x, y;
    grid.get(1, x, y);

    int errors = kiss_label_new(label,
                                window,
                                as_text.c_str(),
                                x,
                                y);

    assert(errors == 0);
}

template <typename... Args>
void init(kiss_window* window, Grid2x2& grid, GenericWindowArgs<bool*, Args...>& ds)
{
    kiss_selectbutton* select = &ds.data_widget;
    bool*              value  = ds.data;

    init_widget_label(window, grid, &ds.label_widget, ds.label);

    int x, y;
    grid.get(1, x, y);

    int errors = kiss_selectbutton_new(select,
                                       window,
                                       x,
                                       y);
    assert(errors == 0);
}

template <typename... Args>
void init(kiss_window* window, Grid2x2& grid, GenericWindowArgs<bool const*, Args...>& ds)
{
    kiss_label* label = &ds.data_widget;
    bool const* value = ds.data;

    init_widget_label(window, grid, &ds.label_widget, ds.label);

    auto as_text = fmt::format("{0}", *value);

    int x, y;
    grid.get(1, x, y);

    int errors = kiss_label_new(label,
                                window,
                                as_text.c_str(),
                                x,
                                y);
    assert(errors == 0);
}

///////////////////////////////////////////////////////////////////////////////


template <typename Tp, typename... Args>
auto window_init(GenericWindowArgs<Tp*, Args...>& ds, kiss_window* window, Grid2x2& grid)
{
    init(window, grid, ds);
    grid.row += 1;

    if constexpr (sizeof...(Args) > 0)
    {
        window_init(ds.args, window, grid);
    }
}


///////////////////////////////////////////////////////////////////////////////

template <typename... Args>
void draw(SDL_Renderer* renderer, GenericWindowArgs<bool*, Args...>& ds)
{
    kiss_label_draw(&ds.label_widget, renderer);
    kiss_selectbutton_draw(&ds.data_widget, renderer);
}

template <typename... Args>
void draw(SDL_Renderer* renderer, GenericWindowArgs<bool const*, Args...>& ds)
{
    kiss_label_draw(&ds.label_widget, renderer);
    kiss_label_draw(&ds.data_widget, renderer);
}

template <typename... Args>
void draw(SDL_Renderer* renderer, GenericWindowArgs<float*, Args...>& ds)
{
    kiss_label_draw(&ds.label_widget, renderer);
    kiss_entry_draw(&ds.data_widget.k, renderer);
}

template <typename... Args>
void draw(SDL_Renderer* renderer, GenericWindowArgs<float const*, Args...>& ds)
{
    kiss_label_draw(&ds.label_widget, renderer);
    kiss_label_draw(&ds.data_widget, renderer);
}


template <typename Tp, typename... Args>
auto window_draw(GenericWindowArgs<Tp, Args...>& ds, SDL_Renderer* renderer)
{
    draw(renderer, ds);

    if constexpr (sizeof...(Args) > 0)
    {
        window_draw(ds.args, renderer);
    }
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Args>
void update(GenericWindowArgs<bool*, Args...>& ds)
{
    auto& widget = ds.data_widget;
    auto  data   = *ds.data;

    widget.selected = data;
}

template <typename... Args>
void update(GenericWindowArgs<bool const*, Args...>& ds)
{
    auto& widget = ds.data_widget;
    auto  data   = *ds.data;

    auto as_string = fmt::format("{}", data);

    strncpy(widget.text,
            as_string.c_str(),
            KISS_MAX_LENGTH);
}

template <typename... Args>
void update(GenericWindowArgs<float*, Args...>& ds)
{
    auto& widget = ds.data_widget;
    auto  data   = *ds.data;

    auto as_string = fmt::format("{}", data);

    strncpy(widget.k.text,
            as_string.c_str(),
            KISS_MAX_LENGTH);
}

template <typename... Args>
void update(GenericWindowArgs<float const*, Args...>& ds)
{
    kiss_label& widget = ds.data_widget;
    auto        data   = *ds.data;

    auto as_string = fmt::format("{}", data);

    strncpy(widget.text,
            as_string.c_str(),
            KISS_MAX_LENGTH);
}


template <typename Tp, typename... Args>
auto window_update(GenericWindowArgs<Tp*, Args...>& ds)
{
    update(ds);

    if constexpr (sizeof...(Args) > 0)
    {
        window_update(ds.args);
    }
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Args>
auto handle_event(SDL_Event* event, int* draw, GenericWindowArgs<bool*, Args...>& ds)
{
    auto& widget = ds.data_widget;

    if (kiss_selectbutton_event(&widget, event, draw))
    {
        auto* choice = ds.data;
        *choice      = widget.selected;
    }
}

template <typename... Args>
auto handle_event(SDL_Event* event, int* draw, GenericWindowArgs<bool const*, Args...>& ds)
{
}

template <typename... Args>
auto handle_event(SDL_Event* event, int* draw, GenericWindowArgs<float*, Args...>& ds)
{
    auto& widget = ds.data_widget;

    if (kiss_entry_event(&widget.k, event, draw))
    {
        auto* number = ds.data;
        char* text   = &widget.k.text[0];

        // TODO: Validation.
        printf("Validating float\n");
        *number = std::stof(text);
    }
}

template <typename... Args>
auto handle_event(SDL_Event* event, int* draw, GenericWindowArgs<float const*, Args...>& ds)
{
}

template <typename Tp, typename... Args>
auto editor_handle_events(GenericWindowArgs<Tp, Args...>& ds, SDL_Event* event, int* draw)
{
    handle_event(event, draw, ds);

    if constexpr (sizeof...(Args) > 0)
    {
        editor_handle_events(ds.args, event, draw);
    }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Tp, typename... Args>
auto window_render(SDL_Renderer* renderer, SDL_Texture* texture, kiss_window* editor_window, GenericWindowArgs<Tp, Args...>& ds)
{
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
    SDL_RenderClear(renderer);

    kiss_window_draw(editor_window, renderer);
    window_draw(ds, renderer);

    SDL_SetRenderTarget(renderer, nullptr);
}

///////////////////////////////////////////////////////////////////////////////

#endif // PROPERTY_EDITOR_HPP