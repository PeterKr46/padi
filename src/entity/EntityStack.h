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
        size_t populate(padi::Map const* map, sf::VertexArray & array, size_t vertexOffset, uint8_t frame) const override;
        [[nodiscard]] size_t numQuads() const override;


        size_t m_stackSize{3};
    };

} // padi
