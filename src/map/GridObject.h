//
// Created by Peter on 02/05/2022.
//

#pragma once

#include <cstdint>
#include "SFML/System/Vector2.hpp"
#include "SFML/Graphics/VertexArray.hpp"

namespace padi {

    class GridPlaceable {
    public:
        explicit GridPlaceable(sf::Vector2i const &pos) : m_position(pos) {}

        [[nodiscard]] sf::Vector2i getPosition() const { return m_position; }

    private:
        friend class Map;

        sf::Vector2i m_position{0, 0};
    };

    class GridObject : public GridPlaceable {
    public:
        explicit GridObject(sf::Vector2i const &pos) : GridPlaceable(pos) {}

        virtual size_t
        populate(padi::Map const *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const = 0;
    };
}

