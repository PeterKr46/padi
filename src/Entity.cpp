//
// Created by Peter on 28/04/2022.
//

#include "Entity.h"

namespace padi {
    Entity::Entity(const sf::Vector2i &pos)
            : GridPlaceable(pos) {

    }

    void Entity::populate(Map const *context, sf::Vertex *quad) const {
        sf::Vector2f anchor = context->mapTilePosToWorld(getPosition());
        sf::Vector2i size = getSize();
        quad[0].position = anchor + sf::Vector2f(-size.x / 2, -size.y / 2);
        quad[1].position = anchor + sf::Vector2f(size.x / 2, -size.y / 2);
        quad[2].position = anchor + sf::Vector2f(size.x / 2, size.y / 2);
        quad[3].position = anchor + sf::Vector2f(-size.x / 2, size.y / 2);

    }
}