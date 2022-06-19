//
// Created by Peter on 28/04/2022.
//

#include "Entity.h"
#include "../Constants.h"

namespace padi {
    Entity::Entity(sf::Vector2i const& pos)
            : padi::GridObject(pos) {

    }

    size_t Entity::numQuads() const {
        return 1;
    }

    void Entity::setVerticalOffset(float vo) {
        m_verticalOffset = vo;
    }

    float Entity::getVerticalOffset() const {
        return m_verticalOffset;
    }
}