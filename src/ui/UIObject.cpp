//
// Created by Peter on 02/05/2022.
//

#include "UIObject.h"

#include <utility>
#include "../Constants.h"

namespace padi {
    UIObject::UIObject(const sf::IntRect &rect) {

    }

    size_t
    UIObject::populate(const padi::UIContext *ctx, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) {
        sf::Vertex *quad = &array[vertexOffset];
        sf::Vector2f texSize{padi::UIPadding_px * 2, padi::UIPadding_px * 2};
        sf::Vector2f texOffset{0, 0};
        if (m_animation) {
            texSize = sf::Vector2f(m_animation->getResolution());
            texOffset = (*m_animation)[frame];
        }
        quad[0].position.x = quad[3].position.x = m_bound.left;
        quad[1].position.x = quad[2].position.x = m_bound.left + m_bound.width;

        quad[0].position.y = quad[1].position.y  = m_bound.top;
        quad[2].position.y = quad[3].position.y = m_bound.top + m_bound.height;

        quad[0].texCoords = texOffset;
        quad[1].texCoords = texOffset + sf::Vector2f(texSize.x, 0);
        quad[2].texCoords = texOffset + texSize;
        quad[3].texCoords = texOffset + sf::Vector2f(0, texSize.y);

        quad[0].color = quad[1].color = quad[2].color = quad[3].color = m_color;

        return 4;
    }

    UIObject::UIObject() = default;

    size_t UIObject::numQuads() const {
        return 1;
    }
} // padi