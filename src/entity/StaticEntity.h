//
// Created by Peter on 03/05/2022.
//

#pragma once

#include "Entity.h"
#include <SFML/System/Vector2.hpp>

namespace padi {

    class Animation;

    class StaticEntity
            : public padi::Entity {
    public:
        explicit StaticEntity(const sf::Vector2i &pos, uint32_t type = 0);

        size_t populate(padi::Map const* map, sf::VertexArray & array, size_t vertexOffset, uint8_t frame, float tileVerticalOffset) const override;

        [[nodiscard]] sf::Vector2i getSize() const override;


        sf::Color m_color{255, 255, 255};
        std::shared_ptr<padi::Animation> m_animation;
    };
}