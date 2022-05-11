//
// Created by Peter on 03/05/2022.
//

#include "StaticEntity.h"
#include "../media/Animation.h"
#include "../Constants.h"

namespace padi {

    padi::StaticEntity::StaticEntity(const sf::Vector2i &pos) : Entity(pos) {

    }

    sf::Vector2i padi::StaticEntity::getSize() const {
        return m_animation->getResolution();
    }

    size_t padi::StaticEntity::populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const {
        sf::Vector2f size{getSize()};
        auto pVertex = &array[vertexOffset];

        sf::Vector2f anchor = map->mapTilePosToWorld(getPosition());
        float verticalOffset = std::min(float(padi::TileSize.y), size.y) / 2;
        pVertex[0].position = anchor + sf::Vector2f(-size.x / 2, verticalOffset - size.y);
        pVertex[1].position = anchor + sf::Vector2f(size.x / 2,  verticalOffset - size.y);
        pVertex[2].position = anchor + sf::Vector2f(size.x / 2,   verticalOffset);
        pVertex[3].position = anchor + sf::Vector2f(-size.x / 2,  verticalOffset);

        sf::Vector2f texCoordAnchor = (*m_animation)[frame];
        pVertex[0].texCoords = texCoordAnchor;
        pVertex[1].texCoords = texCoordAnchor + sf::Vector2f(size.x, 0);
        pVertex[2].texCoords = texCoordAnchor + sf::Vector2f(size);
        pVertex[3].texCoords = texCoordAnchor + sf::Vector2f(0, size.y);

        for (int i = 0; i < 4; ++i) pVertex[i].color = m_color;

        return 4;
    }
}