#include "collision/core.hpp"
#include "entity/core.hpp"
#include <vector>

namespace collision {

void detect_soft_collisions(entity::Player&                    player,
                            std::vector<entity::EntityStatic>& game_entities,
                            std::vector<SDL_FRect>&            soft_boundaries)
{
    std::vector<int> interacted_with;

    for (int entity_idx = 0;
         (entity_idx < game_entities.size());
         ++entity_idx)
    {
        auto& boundary = soft_boundaries[entity_idx];
        auto& entity   = game_entities[entity_idx];

        bool collided = collision::is_point_in_rect(player.e.X[0][0],
                                                    player.e.X[0][1],
                                                    boundary);

        if (collided && entity.alive)
        {
            switch (entity.kind_of)
            {
            case entity::EntityKinds::Food: {
                printf("Collided with food\n");
                entity.alive = false;
                player.restore();
                break;
            }
            case entity::EntityKinds::Boundary: {
                break;
            }
            }
        }
    }
}

}
