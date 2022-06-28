//
// Created by Peter on 03/05/2022.
//

#include "Tile.h"

#include <utility>
#include "Map.h"
#include "../Constants.h"

namespace padi {
    Tile::Tile(const sf::Vector2i &pos) : GridObject(pos) {
        m_detail = size_t(getPosition().x + getPosition().y) % 2;
    }

    size_t Tile::populate(const padi::Map *context, sf::VertexArray &array, size_t vertexOffset, uint8_t frame,
                          float tileVerticalOffset) const {
        sf::Vertex *quad = &array[vertexOffset];
        sf::Vector2f anchor = padi::Map::mapTilePosToWorld(getPosition());
        anchor.y += m_verticalOffset;

        auto detail = float(m_detail);
        uint16_t csum = m_color.r + m_color.g + m_color.b;
        if  (csum <= 96) detail = 2;
        else if(csum >= 700) detail = 3;

        sf::Vector2f tileSize(padi::TileSize);
        quad[0].position = anchor + sf::Vector2f(-tileSize.x / 2, -tileSize.y / 2);
        quad[1].position = anchor + sf::Vector2f(tileSize.x / 2, -tileSize.y / 2);
        quad[2].position = anchor + sf::Vector2f(tileSize.x / 2, tileSize.y / 2);
        quad[3].position = anchor + sf::Vector2f(-tileSize.x / 2, tileSize.y / 2);
        quad[0].color = quad[1].color = quad[2].color = quad[3].color = m_color;
        // TODO hardcoded lol
        quad[0].texCoords = sf::Vector2f(992 + 0, 0     + detail * tileSize.y);
        quad[1].texCoords = sf::Vector2f(992 + 32, 0    + detail * tileSize.y);
        quad[2].texCoords = sf::Vector2f(992 + 32, 32   + detail * tileSize.y);
        quad[3].texCoords = sf::Vector2f(992 + 0, 32    + detail * tileSize.y);

        size_t offset = 0;
        if (m_decoration) {
            offset += m_decoration->populate(context, array, vertexOffset + 4, frame, m_verticalOffset);
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

    float Tile::getVerticalOffset() const {
        return m_verticalOffset;
    }

    void Tile::setVerticalOffset(float vo) {
        m_verticalOffset = vo;
    }

    std::shared_ptr<padi::TileDecoration> Tile::getDecoration() const {
        return m_decoration;
    }

    void Tile::setDecoration(std::shared_ptr<padi::TileDecoration> decor) {
        m_decoration = std::move(decor);
    }

    size_t Tile::numQuads() const {
        return m_decoration ? 2 : 1;
    }

    Tile::Tile(int x, int y)
            : GridObject({x, y}) {

    }

    void Tile::lerpColor(const sf::Color &c, float power) {
        const auto *rawTarget = reinterpret_cast<const uint8_t *>(&c);
        auto *rawStatus = reinterpret_cast<uint8_t *>(&m_color);
        float delta;
        for (int i = 0; i < 3; ++i) {
            delta = float(rawTarget[i] - rawStatus[i]);
            rawStatus[i] += int16_t(ceil(delta * power));
        }
    }

    void Tile::lerpAdditiveColor(const sf::Color &c, float power) {
        const auto *rawTarget = reinterpret_cast<const uint8_t *>(&c);
        auto *rawStatus = reinterpret_cast<uint8_t *>(&m_color);
        float delta;
        for (int i = 0; i < 3; ++i) {
            delta = std::max(0.0f, float(rawTarget[i] - rawStatus[i]));
            rawStatus[i] += int16_t(ceil(delta * power));
        }
    }
} // padi