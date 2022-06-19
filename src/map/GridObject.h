//
// Created by Peter on 02/05/2022.
//

#pragma once

#include <cstdint>
#include <memory>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace padi {

    class Map;

    class GridObject {
    public:
        explicit GridObject(sf::Vector2i const &pos) : m_position(pos) {}


        [[nodiscard]] virtual size_t numQuads() const { return 1; };

        virtual size_t populate(padi::Map const *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame, float tileVerticalOffset) const = 0;

        [[nodiscard]] sf::Vector2i getPosition() const { return m_position; }

    private:
        friend class Map;
        sf::Vector2i m_position{0, 0};
    };
}

