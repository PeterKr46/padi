//
// Created by Peter on 17/06/2022.
//

#pragma once

#include "GridObject.h"
#include "../map/Map.h"
#include "../media/Animation.h"

namespace padi {

    class TileDecoration : public padi::GridObject {
    public:
        explicit TileDecoration(sf::Vector2i const &pos, std::shared_ptr<padi::Animation> anim = {nullptr});

        size_t populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const override;

        std::shared_ptr<padi::Animation> m_animation;
        float m_verticalOffset{0};
    };

} // content
