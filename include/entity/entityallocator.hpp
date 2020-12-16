#pragma once
#include "entity/entity.hpp"
#include <array>

namespace entity {

struct Allocator {
    // Note(DW): Do not make these dynamically reallocating arrays.
    // Entities take a raw pointer to elements in these array that will become invalid if the array reallocates.
    struct {
        std::array<entity::Entity, 100> simulated;
        std::array<entity::Entity, 100> interpolated;
        std::size_t                     size;
    } pos;

    struct {
        std::array<entity::EntityRotation, 100> simulated;
        std::array<entity::EntityRotation, 100> interpolated;
        std::size_t                             size;
    } rot;
};

inline Allocator make_entity_alloca()
{
    Allocator alloca;
    alloca.pos.size = 0;
    alloca.rot.size = 0;
    return alloca;
}

inline int reserve(Allocator&       alloca,
                   entity::Entity*& simu,
                   entity::Entity*& rend)
{
    int index = alloca.pos.size;
    simu      = &alloca.pos.simulated[index];
    rend      = &alloca.pos.interpolated[index];

    alloca.pos.size += 1;
    return index;
}

inline int reserve(Allocator&               alloca,
                   entity::EntityRotation*& simu,
                   entity::EntityRotation*& rend)
{
    int index = alloca.rot.size;
    simu      = &alloca.rot.simulated[index];
    rend      = &alloca.rot.interpolated[index];

    alloca.rot.size += 1;
    return index;
}

inline void interpolate(entity::Allocator& alloca, float dt)
{
    for (std::size_t i = 0; i < alloca.pos.size; ++i)
    {
        auto* e = &alloca.pos.interpolated[i];
        entity::integrate(e, dt);
    }

    for (std::size_t i = 0; i < alloca.rot.size; ++i)
    {
        auto* e = &alloca.rot.interpolated[i];
        entity::integrate(e, dt);
    }
}

inline void update(entity::Allocator& alloca)
{
    alloca.pos.interpolated = alloca.pos.simulated;
    alloca.rot.interpolated = alloca.rot.simulated;
}

} // namespace entity
