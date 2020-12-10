#pragma once
#include "entity/entity.hpp"
#include <array>

namespace entity {

struct Allocator {
    // Note(DW): Do not make these dynamically reallocating arrays.
    // Entities take a raw pointer to elements in these array that will become invalid if the array reallocates.
    std::array<entity::Entity, 100> data;
    std::array<entity::Entity, 100> interpolated;
    std::size_t                     size{};
};

inline Allocator make_entity_alloca()
{
    Allocator alloca;
    return alloca;
}

inline void reserve(Allocator& alloca, int& index)
{
    index = alloca.size;
    alloca.size++;
}

// void integrate(EntityAllocator& alloca, float dt)
// {
//     for (auto& e : alloca.data)
//     {
//         entity::integrate(e, dt);
//     }
// }

inline void interpolate(Allocator& alloca, float dt)
{
    for (auto& e : alloca.interpolated)
    {
        entity::integrate(&e, dt);
    }
}

inline void update(Allocator& alloca)
{
    alloca.interpolated = alloca.data;
}

} // namespace entity
