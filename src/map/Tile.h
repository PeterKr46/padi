//
// Created by Peter on 03/05/2022.
//
#pragma once

#include "SFML/Graphics/Color.hpp"
#include "GridObject.h"

namespace padi {

    class Map;

    class Tile
            : public GridObject {
    public:
        explicit Tile(sf::Vector2i const &pos);

        size_t populate(padi::Map const *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const override;

        size_t m_detail{1};
        sf::Color m_color{255, 255, 255};
    };

} // padi
