//
// Created by Peter on 03/05/2022.
//

#include "Tile.h"
#include "Map.h"

namespace padi {
    Tile::Tile(const sf::Vector2i &pos) : GridObject(pos) {

    }

    size_t Tile::populate(const padi::Map *context, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) {
        if (m_cache.dirty) {
            m_cache.dirty = false;
            sf::Vector2f anchor = context->mapTilePosToWorld(getPosition());
            //+ (m_detail != 0 ? sf::Vector2f(0,0) : sf::Vector2f((frame + getPosition().x + getPosition().y) / 5 % 2, (frame + getPosition().x + getPosition().y) / 5 % 2));
            //+ (m_color.r + m_color.g + m_color.b > 96 ? sf::Vector2f(0, -m_verticalOffset)
            //                                          : sf::Vector2f(0, -m_verticalOffset +
            //                                                            (frame + getPosition().x +
            //                                                             getPosition().y) / 2 % 2));
            sf::Vector2f tileSize(context->getTileSize());
            m_cache.vertex[0].position = anchor + sf::Vector2f(-tileSize.x / 2, -tileSize.y / 2);
            m_cache.vertex[1].position = anchor + sf::Vector2f(tileSize.x / 2, -tileSize.y / 2);
            m_cache.vertex[2].position = anchor + sf::Vector2f(tileSize.x / 2, tileSize.y / 2);
            m_cache.vertex[3].position = anchor + sf::Vector2f(-tileSize.x / 2, tileSize.y / 2);


            m_cache.vertex[0].texCoords = sf::Vector2f(128 + 0, 0 + m_detail * tileSize.y);
            m_cache.vertex[1].texCoords = sf::Vector2f(128 + 32, 0 + m_detail * tileSize.y);
            m_cache.vertex[2].texCoords = sf::Vector2f(128 + 32, 32 + m_detail * tileSize.y);
            m_cache.vertex[3].texCoords = sf::Vector2f(128 + 0, 32 + m_detail * tileSize.y);
        }
        memcpy(&array[vertexOffset], &m_cache.vertex, sizeof(sf::Vertex[4]));
        return 4;
    }

    void Tile::setColor(sf::Color const &c) {
        for (auto &v: m_cache.vertex) {
            v.color = c;
        }
    }

    sf::Color Tile::getColor() const {
        return m_cache.vertex[0].color;
    }

    int32_t Tile::getVerticalOffset() const {
        return m_verticalOffset;
    }

    void Tile::setVerticalOffset(int32_t vo) {
        m_verticalOffset = vo;
    }
} // padi