#ifndef UNTITLED_SERIALISATION_HPP
#define UNTITLED_SERIALISATION_HPP

#include "gameevents.h"
#include <filesystem>

namespace serialisation {
// auto run_msgpack_example() -> int;
// auto run_load_save_example(std::filesystem::path const& exe_base_dir, GameEvents& events) -> void;

auto save(std::filesystem::path const&, GameEvents&) -> void;
auto load(std::filesystem::path const&, GameEvents&) -> void;
}

#endif