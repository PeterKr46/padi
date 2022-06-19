//
// Created by Peter on 28/04/2022.
//

#pragma once

#include "../map/Map.h"

namespace padi {
    class Entity : public padi::GridObject {
    public:
        explicit Entity(sf::Vector2i const &pos);

        size_t populate(padi::Map const* map, sf::VertexArray & array, size_t vertexOffset, uint8_t frame, float tileVerticalOffset) const = 0;
        [[nodiscard]] size_t numQuads() const override;

        [[nodiscard]] virtual sf::Vector2i getSize() const = 0;

        void setVerticalOffset(float vo);
        [[nodiscard]] float getVerticalOffset() const;
    private:
        float m_verticalOffset{0};
    };
}