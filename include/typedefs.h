#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <array>
#include <inttypes.h>
#include <string>
#include <vector>

using int8   = int8_t;
using int16  = int16_t;
using int32  = int32_t;
using int64  = int64_t;
using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using byte  = int8_t;
using ubyte = uint8_t;

template <typename Tp, std::size_t Nm>
using array = std::array<Tp, Nm>;

template <typename Tp>
using vector = std::vector<Tp>;

using sting = std::string;

#endif
