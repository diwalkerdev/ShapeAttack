#include "entity/entity.hpp"
#include "entity/entityallocator.hpp"
#include "linalg/matrix.hpp"
#include <stdio.h>

/*
void simulate(EntityAllocator& alloca, float dt)
{
    // This is more complicated than a simple integration.
    // integrate(alloca, dt);

    alloca.interpolated = alloca.data;
}
*/

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
namespace entity {

struct Spring {
    entity::Entity* e;
    entity::Entity* r;
};

Spring make_spring(entity::Allocator& alloca, float x0, float y0)
{
    int index;
    reserve(alloca, index);

    Spring spring;
    spring.e = &alloca.data[index];
    spring.r = &alloca.interpolated[index];

    float const k = 0.9;
    float const b = 0.2;
    float const m = 1.0;

    spring.e->X[0][0] = x0;
    spring.e->X[0][1] = y0;

    // This spring doesn't have Xy states.
    spring.e->A = {{{0, 1}, {-k / m, -b / m}}};
    spring.e->B = {{{0, 0}, {1 / m, 0}}};

    return spring;
}

void set_input(Spring& spring, float x, float y)
{
    spring.e->u[0][0] = x;
    spring.e->u[0][1] = y;
}

void simulate(Spring& spring, float dt)
{
}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void investigate_addresses_of_state_space_variables()
{
    // It will only make sense to pack entities into an array if the state
    // space variables themselves are packed.

    entity::Entity e;
    float*         a_0 = &e.X[0][0];
    float*         a_3 = &e.X[1][1];
    float*         x_d = &e.Xdot[0][0];
    float*         b_0 = &e.B[0][0];

    printf("a_0 - a_0: %ld\n", (a_0 - a_0) * sizeof(float));
    printf("a_3 - a_0: %ld\n", (a_3 - a_0) * sizeof(float));
    printf("x_d - a_0: %ld\n", (x_d - a_0) * sizeof(float));
    printf("b_0 - a_0: %ld\n", (b_0 - a_0) * sizeof(float));
}

void test_reserve_allocates_one_entity()
{
    // Check everything is empty on initialisation.
    auto entity_alloca = make_entity_alloca();
    assert(entity_alloca.data.size() == 0);

    // Check first reservation.
    int index_1;
    reserve(entity_alloca, index_1);

    assert(entity_alloca.data.size() == 1);
    assert(index_1 == 0);

    // Check second reservation.
    int index_2;
    reserve(entity_alloca, index_2);

    assert(entity_alloca.data.size() == 2);
    assert(index_2 == 1);
}

void test_integration_of_serveral_entities()
{
    auto entity_alloca = make_entity_alloca();

    // Probably want to create a player, bullet and a spring?
    float x0, y0;
    // auto  player = entity::make_player(entity_alloca, x0, y0);

    auto spring = entity::make_spring(entity_alloca, x0, y0);

    float ux, uy;
    //set_input(player, ux, uy);
    set_input(spring, ux, uy);

    float dt = 0.333;
    // Don't think this is sufficient. I think you need to simulate the concrete types and they
    // might need extra information like boundaries.
    // simulate(entity_alloca, dt);

    // Very verbose.
    simulate(spring, dt);
    // simulate(game_entities, boundaries, dt);

    // set_input(player, 0, 0);
    set_input(spring, 0, 0);

    dt = 0.1;
    interpolate(entity_alloca, dt);


    float current_time;
    float accumulator;

    float new_time   = SDL_GetTicks();
    float frame_time = new_time - current_time;

    // What does this line do?
    if (frame_time > 0.25)
    {
        frame_time = 0.25;
    }

    current_time = new_time;

    while (accumulator >= dt)
    {
        simulate(spring, dt);
        accumulator -= dt;
    }

    ::interpolate(entity_alloca, accumulator);
    render();
}


#ifdef TEST_ENTITIES

int main()
{
    investigate_addresses_of_state_space_variables();
    test_reserve_allocates_one_entity();
    printf("Entities Tests Passed");
    return 0;
}

#endif
