//
// Created by Peter on 02/07/2022.
//

#pragma once

#include "../../entity/StaticEntity.h"

namespace padi::content {

    class Event : public StaticEntity {
    public:
        Event(sf::Vector2i const& pos);

        size_t populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame, float tileVerticalOffset) const override;

    };

} // content
