//
// Created by Peter on 05/05/2022.
//

#include "Switch.h"
#include "UIContext.h"
#include "../Controls.h"

namespace padi {

    Switch::Switch(padi::UIContext *context) {
        m_animation = context->getApollo()->lookupAnim("switch");
        m_bound.left = 32;
    }

    size_t Switch::populate(const padi::UIContext *ctx, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) {

        if (ctx->isFocused(shared_from_this())) {
            if(padi::Controls::isKeyDown(sf::Keyboard::Space)) {
                m_state = !m_state;
                if(m_state) {
                    m_soundSource.setBuffer(*ctx->getApollo()->lookupAudio("switch_on"));
                } else {
                    m_soundSource.setBuffer(*ctx->getApollo()->lookupAudio("switch_off"));
                }
                m_soundSource.play();
            }
            m_color = sf::Color::Yellow;
        } else {
            m_color = sf::Color::White;
        }
        return UIObject::populate(ctx, array, vertexOffset, m_state);
    }
} // padi