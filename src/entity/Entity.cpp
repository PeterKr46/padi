//
// Created by Peter on 28/04/2022.
//

#include "Entity.h"
#include "../Constants.h"

namespace padi {
    Entity::Entity(sf::Vector2i const& pos)
            : padi::GridObject(pos) {

    }

    size_t
    Entity::populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const {
        auto quad = &array[vertexOffset];
        sf::Vector2f size{getSize()};
        sf::Vector2f anchor = map->mapTilePosToWorld(getPosition());
        float verticalOffset = std::min(float(padi::TileSize.y), size.y) / 2;
        quad[0].position = anchor + sf::Vector2f(-size.x / 2, verticalOffset - size.y);
        quad[1].position = anchor + sf::Vector2f(size.x / 2,  verticalOffset - size.y);
        quad[2].position = anchor + sf::Vector2f(size.x / 2,   verticalOffset);
        quad[3].position = anchor + sf::Vector2f(-size.x / 2,  verticalOffset);
        return 4;
    }

    size_t Entity::numQuads() const {
        return 1;
    }

    void Entity::setVerticalOffset(float vo) {
        m_verticalOffset = vo;
    }

    float Entity::getVerticalOffset() const {
        return m_verticalOffset;
    }
}