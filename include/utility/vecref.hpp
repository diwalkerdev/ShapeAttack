#pragma once

#include <vector>

// Provides a "reference" to an element in a vector.
// vecref solves:
//  - Storing references to vector elements is unsafe, since vectors can
// reallocate.
//  - Storing the reference to the array and an index adds overhead
// and exposes the vector.
//
template <typename Tp>
struct vecref {
    using container = std::vector<Tp>;
public:
    // Ctor
    vecref(container& vec, std::size_t index) noexcept
        : vec(vec)
        , index(index)
    {
    }

    // Copy
    vecref(vecref const& other) noexcept = default;

    auto operator=(vecref const& other) noexcept -> vecref<Tp>&
    {
        vec   = other.vec;
        index = other.index;
        return *this;
    }

    // Move
    vecref(vecref&& other) noexcept = default;

    auto operator=(vecref&& other) noexcept -> vecref<Tp>& = default;

    // Accessors
    auto get() noexcept -> Tp& { return vec[index]; }

private:
    container& vec;
    std::size_t index;
};

template <typename Tp>
vecref(std::vector<Tp>& vec, int index) -> vecref<Tp>;

