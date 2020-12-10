#pragma once
#include "entity/entity.hpp"

namespace entity {

inline auto make_food()
{
    EntityStatic food;
    food.rect        = {200.f, 100.f, 40.f, 40.f};
    food.restitution = 1.f;
    food.alive       = true;
    food.kind_of     = EntityKinds::Food;
    return food;
}

inline auto make_wall()
{
    EntityStatic wall;
    wall.rect        = {300.f, 200.f, 40.f, 40.f};
    wall.restitution = 1.f;
    wall.alive       = true;
    wall.kind_of     = EntityKinds::Boundary;
    return wall;
}

}
