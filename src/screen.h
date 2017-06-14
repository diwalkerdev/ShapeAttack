#ifndef SCREEN_H
#define SCREEN_H

struct Screen {
  static constexpr const auto width = 640;
  static constexpr const auto height = 480;
  static constexpr const SDL_Point center{
    Screen::width / 2, 
    Screen::height / 2
  };
};

#endif  // SCREEN_H