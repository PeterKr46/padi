//
// Created by Peter on 03/05/2022.
//

#include "Tile.h"
#include "Map.h"

namespace padi {
    Tile::Tile(const sf::Vector2i &pos) : GridObject(pos) {

    }

    size_t Tile::populate(const padi::Map *context, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const {
        sf::Vertex* quad = &array[vertexOffset];

        sf::Vector2f anchor = context->mapTilePosToWorld(getPosition())
                //+ (m_detail != 0 ? sf::Vector2f(0,0) : sf::Vector2f((frame + getPosition().x + getPosition().y) / 5 % 2, (frame + getPosition().x + getPosition().y) / 5 % 2));
                + (m_color.r + m_color.g + m_color.b > 96 ? sf::Vector2f(0,0) : sf::Vector2f(0, (frame + getPosition().x + getPosition().y) / 2 % 2));
        sf::Vector2f tileSize = {static_cast<float>(context->getTileSize().x), static_cast<float>(context->getTileSize().y)};
        quad[0].position = anchor+sf::Vector2f(-tileSize.x / 2,-tileSize.y / 2);
        quad[1].position = anchor+sf::Vector2f(tileSize.x / 2,-tileSize.y / 2);
        quad[2].position = anchor+sf::Vector2f(tileSize.x / 2,tileSize.y / 2);
        quad[3].position = anchor+sf::Vector2f(-tileSize.x / 2,tileSize.y / 2);
        quad[0].color = m_color;
        quad[1].color = m_color;
        quad[2].color = m_color;
        quad[3].color = m_color;


        quad[0].texCoords = sf::Vector2f(128 + 0, 0 + m_detail * tileSize.y);
        quad[1].texCoords = sf::Vector2f(128 + 32, 0 + m_detail * tileSize.y);
        quad[2].texCoords = sf::Vector2f(128 + 32, 32 + m_detail * tileSize.y);
        quad[3].texCoords = sf::Vector2f(128 + 0, 32 + m_detail * tileSize.y);
        return 4;
    }
} // padi