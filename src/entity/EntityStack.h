//
// Created by Peter on 03/05/2022.
//

#pragma once

#include "Entity.h"
#include "StaticEntity.h"

namespace padi {

    class EntityStack : public padi::StaticEntity {
    public:
        explicit EntityStack(const sf::Vector2i &pos);

        size_t
        populate(padi::Map const *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const override;

        [[nodiscard]] size_t numQuads() const override;


        size_t m_stackSize{3};
        float m_verticalStep{0};
    };

    class EntityColumn : public padi::StaticEntity {
    public:
        explicit EntityColumn(const sf::Vector2i & pos);

        size_t populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const override;

        float   m_verticalStep{0};
        size_t  m_stackSize{3};
        std::shared_ptr<padi::Animation> m_stackAnimation{nullptr};

    };

} // padi
