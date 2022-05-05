//
// Created by Peter on 04/05/2022.
//

#include "Button.h"
#include "UIContext.h"
#include "../media/Apollo.h"
#include "../Controls.h"

namespace padi {
    Button::Button(padi::UIContext const *ctx) {
        m_animation = ctx->getApollo()->lookupAnim("scalable_button");
    }

    size_t Button::populate(const padi::UIContext *ctx, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) {
        frame = 0;
        if (ctx->isFocused(shared_from_this())) {
            frame = 1 + padi::Controls::pollKeyState(sf::Keyboard::Space);
            m_color = sf::Color::Yellow;
        } else {
            m_color = sf::Color::White;
        }
        return ScalableUIObject::populate(ctx, array, vertexOffset, frame);
    }
} // padi