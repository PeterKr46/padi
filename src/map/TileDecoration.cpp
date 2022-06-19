//
// Created by Peter on 17/06/2022.
//

#include "TileDecoration.h"

#include <utility>
#include "../Constants.h"

namespace padi {

    TileDecoration::TileDecoration(const sf::Vector2i &pos, std::shared_ptr<padi::Animation> anim)
            : padi::GridObject(pos),
              m_animation(std::move(anim)) {

    }

    size_t TileDecoration::populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset,
                                    uint8_t frame, float tileVerticalOffset) const {
        sf::Vector2f size{m_animation->getResolution()};
        auto pVertex = &array[vertexOffset];

        sf::Vector2f anchor = padi::Map::mapTilePosToWorld(getPosition());
        float verticalOffset = tileVerticalOffset + std::min(float(padi::TileSize.y), size.y) / 2 - 8;

        pVertex[0].position = anchor + sf::Vector2f(-size.x / 2, verticalOffset - size.y);
        pVertex[1].position = anchor + sf::Vector2f(size.x / 2,  verticalOffset - size.y);
        pVertex[2].position = anchor + sf::Vector2f(size.x / 2,   verticalOffset);
        pVertex[3].position = anchor + sf::Vector2f(-size.x / 2,  verticalOffset);

        sf::Vector2f texCoordAnchor = (*m_animation)[frame];
        pVertex[0].texCoords = texCoordAnchor;
        pVertex[1].texCoords = texCoordAnchor + sf::Vector2f(size.x, 0);
        pVertex[2].texCoords = texCoordAnchor + sf::Vector2f(size);
        pVertex[3].texCoords = texCoordAnchor + sf::Vector2f(0, size.y);

        return 4;
    }

} // content