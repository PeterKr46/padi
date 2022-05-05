//
// Created by Peter on 03/05/2022.
//

#include "EntityStack.h"
#include "../animation/Animation.h"

namespace padi {
    EntityStack::EntityStack(const sf::Vector2i &pos) : StaticEntity(pos) {

    }

    size_t
    EntityStack::populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) {
        sf::Vector2f size{getSize()};

        sf::Vector2f anchor = map->mapTilePosToWorld(getPosition());
        float verticalOffset = std::min(float(map->getTileSize().y), size.y) / 2;
        sf::Vertex* quad;
        sf::Vector2f texCoordAnchor;

        for (size_t i = 0; i < m_stackSize; ++i) {
            quad = &array[vertexOffset + 4*i];
            quad[0].position = anchor + sf::Vector2f(-size.x / 2, verticalOffset - size.y);
            quad[1].position = anchor + sf::Vector2f(size.x / 2, verticalOffset - size.y);
            quad[2].position = anchor + sf::Vector2f(size.x / 2, verticalOffset);
            quad[3].position = anchor + sf::Vector2f(-size.x / 2, verticalOffset);

            texCoordAnchor = (*m_animation)[frame];
            quad[0].texCoords = texCoordAnchor;
            quad[1].texCoords = texCoordAnchor + sf::Vector2f(size.x, 0);
            quad[2].texCoords = texCoordAnchor + sf::Vector2f(size);
            quad[3].texCoords = texCoordAnchor + sf::Vector2f(0, size.y);

            for (int v = 0; v < 4; ++v) quad[v].color = m_color;

            verticalOffset -= size.y;
        }
        return m_stackSize * 4;
    }
} // padi