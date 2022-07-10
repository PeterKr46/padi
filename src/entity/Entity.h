//
// Created by Peter on 28/04/2022.
//

#pragma once

#include "../map/Map.h"

namespace padi {
    class Entity : public padi::GridObject {
    public:
        explicit Entity(sf::Vector2i const &pos, uint32_t type);

        size_t populate(padi::Map const *map,
                        sf::VertexArray &array,
                        size_t vertexOffset,
                        uint8_t frame,
                        float tileVerticalOffset) const override = 0;

        [[nodiscard]] size_t numQuads() const override;

        [[nodiscard]] virtual sf::Vector2i getSize() const = 0;

        void setVerticalOffset(float vo);

        [[nodiscard]] float getVerticalOffset() const;

        [[nodiscard]] uint32_t getType() const;

    protected:
        const uint32_t m_entityType;
    private:
        float m_verticalOffset{0};
    };

    enum EntityType : uint32_t {
        BLANK = 0,
        CURSOR = 1u,
        LIVING = CURSOR << 1,
        BEACON = LIVING << 1,
        PLAYER = BEACON << 1,
        EVENT  = PLAYER << 1,
        INCOMING = EVENT << 1,

        EXPLOSIVE = PLAYER << 8,
    };
}