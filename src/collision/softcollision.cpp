#include "entity.hpp"
#include <vector>

void detect_soft_collisions(Player&                    player,
                            std::vector<EntityStatic>& game_entities,
                            std::vector<SDL_FRect>&    soft_boundaries)
{
    std::vector<int> interacted_with;

    for (int entity_idx = 0;
         (entity_idx < game_entities.size());
         ++entity_idx)
    {
        auto& boundary = soft_boundaries[entity_idx];
        auto& entity   = game_entities[entity_idx];

        bool collided = is_point_in_rect(player.e.X[0][0],
                                         player.e.X[0][1],
                                         boundary);

        if (collided && entity.alive)
        {
            switch (entity.kind_of)
            {
            case EntityKinds::Food: {
                printf("Collided with food\n");
                entity.alive = false;
                player.eat(0.5);
                break;
            }
            }
        }
    }
}