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
    UIObject::populate(const padi::UIContext *ctx, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const {
        sf::Vertex *quad = &array[vertexOffset];
        sf::Vector2f texSize{padi::UIPadding_px * 2, padi::UIPadding_px * 2};
        sf::Vector2f texOffset{0, 0};
        if (m_animation) {
            texSize = sf::Vector2f(m_animation->getResolution());
            texOffset = (*m_animation)[frame];
        }
        float x_anchors[4]{
                m_bound.left,
                m_bound.left + padi::UIPadding_px,
                m_bound.left + m_bound.width - padi::UIPadding_px,
                m_bound.left + m_bound.width};
        float y_anchors[4]{
                m_bound.top,
                m_bound.top + padi::UIPadding_px,
                m_bound.top + m_bound.height - padi::UIPadding_px,
                m_bound.top + m_bound.height};
        float x_tex[4]{
                texOffset.x,
                texOffset.x + padi::UIPadding_px,
                texOffset.x + texSize.x - padi::UIPadding_px,
                texOffset.x + texSize.x};
        float y_tex[4]{
                texOffset.y,
                texOffset.y + padi::UIPadding_px,
                texOffset.y + texSize.y - padi::UIPadding_px,
                texOffset.y + texSize.y};

        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                quad[0].position.x = x_anchors[x];
                quad[0].position.y = y_anchors[y];
                quad[0].texCoords.x = x_tex[x];
                quad[0].texCoords.y = y_tex[y];

                quad[1].position.x = x_anchors[x + 1];
                quad[1].position.y = y_anchors[y];
                quad[1].texCoords.x = x_tex[x + 1];
                quad[1].texCoords.y = y_tex[y];

                quad[2].position.x = x_anchors[x + 1];
                quad[2].position.y = y_anchors[y + 1];
                quad[2].texCoords.x = x_tex[x + 1];
                quad[2].texCoords.y = y_tex[y + 1];

                quad[3].position.x = x_anchors[x];
                quad[3].position.y = y_anchors[y + 1];
                quad[3].texCoords.x = x_tex[x];
                quad[3].texCoords.y = y_tex[y + 1];
                quad[0].color = quad[1].color = quad[2].color = quad[3].color = m_color;
                quad += 4;
            }
        }

        return 9 * 4;
    }

    UIObject::UIObject() = default;

    size_t UIObject::numQuads() const {
        return 9;
    }
} // padi