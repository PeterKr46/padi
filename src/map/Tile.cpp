//
// Created by Peter on 03/05/2022.
//

#include "Tile.h"

#include <utility>
#include "Map.h"

namespace padi {
    Tile::Tile(const sf::Vector2i &pos) : GridObject(pos) {

    }

    size_t Tile::populate(const padi::Map *context, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const {
        sf::Vertex* quad = &array[vertexOffset];
        sf::Vector2f anchor = context->mapTilePosToWorld(getPosition());
        sf::Vector2f tileSize(context->getTileSize());
        quad[0].position = anchor + sf::Vector2f(-tileSize.x / 2, -tileSize.y / 2);
        quad[1].position = anchor + sf::Vector2f(tileSize.x / 2, -tileSize.y / 2);
        quad[2].position = anchor + sf::Vector2f(tileSize.x / 2, tileSize.y / 2);
        quad[3].position = anchor + sf::Vector2f(-tileSize.x / 2, tileSize.y / 2);
        quad[0].color = quad[1].color = quad[2].color = quad[3].color = m_color;


        quad[0].texCoords = sf::Vector2f(128 + 0, 0 + m_detail * tileSize.y);
        quad[1].texCoords = sf::Vector2f(128 + 32, 0 + m_detail * tileSize.y);
        quad[2].texCoords = sf::Vector2f(128 + 32, 32 + m_detail * tileSize.y);
        quad[3].texCoords = sf::Vector2f(128 + 0, 32 + m_detail * tileSize.y);

        size_t offset = 0;
        if(m_decoration) {
            offset += m_decoration->populate(context, array, vertexOffset + 4, frame);
            quad = &array[vertexOffset + 4];
            quad[0].color = quad[1].color = quad[2].color = quad[3].color = m_color;
        }
        return 4 + offset;
    }

    void Tile::setColor(sf::Color const &c) {
        m_color = c;
    }

    sf::Color Tile::getColor() const {
        return m_color;
    }

    int32_t Tile::getVerticalOffset() const {
        return m_verticalOffset;
    }

    void Tile::setVerticalOffset(int32_t vo) {
        m_verticalOffset = vo;
    }

    std::shared_ptr<padi::Entity> Tile::getDecoration() const {
        return m_decoration;
    }

    void Tile::setDecoration(std::shared_ptr<padi::Entity> decor) {
        m_decoration = std::move(decor);
    }

    size_t Tile::numQuads() const {
        return 1 + m_decoration.operator bool();
    }
} // padi