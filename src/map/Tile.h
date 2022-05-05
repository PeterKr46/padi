//
// Created by Peter on 03/05/2022.
//
#pragma once

#include <memory>
#include <SFML/Graphics/Color.hpp>
#include "GridObject.h"
#include "../entity/Entity.h"

namespace padi {

    class Map;

    class Tile
            : public GridObject {
    public:
        explicit Tile(sf::Vector2i const &pos);

        size_t populate(padi::Map const *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const override;

        void setColor(sf::Color const &c);

        [[nodiscard]] sf::Color getColor() const;

        void setVerticalOffset(int32_t vo);

        [[nodiscard]] int32_t getVerticalOffset() const;

        std::shared_ptr<padi::Entity> getDecoration() const;
        void setDecoration(std::shared_ptr<padi::Entity> decor);

        size_t numQuads() const override;

        bool m_walkable{true};

    private:
        size_t m_detail{0};
        int32_t m_verticalOffset{0};
        sf::Color m_color;
        std::shared_ptr<padi::Entity> m_decoration;
    };

} // padi
