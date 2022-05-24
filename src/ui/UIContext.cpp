//
// Created by Peter on 05/05/2022.
//

#include "UIContext.h"
#include "../Controls.h"

#include <SFML/Graphics.hpp>

namespace padi {
    void UIContext::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        states.texture = &m_sprites;

        states.transform.translate(target.getView().getCenter() - target.getView().getSize() / 2.f);
        // TODO
        states.transform.scale(sf::Vector2f(target.getView().getSize().y / 256, target.getView().getSize().y / 256));

        // draw the vertex array
        target.draw(&m_vbo[0], m_numVerts, sf::PrimitiveType::Quads, states);

    }

    size_t UIContext::numQuads() const {
        return m_numVerts / 4;
    }

    void UIContext::init(const std::string &apollo, const std::string &sprite) {
        m_apollo.loadFromFile(apollo);
        m_sprites.loadFromFile(sprite);
    }

    const padi::Apollo *UIContext::getApollo() const {
        return &m_apollo;
    }

    void UIContext::setFocus(size_t elemId) {
        m_focused = elemId;
    }

    bool UIContext::isFocused(size_t elemID) const {
        return m_focused == elemID;
    }

    void UIContext::nextFrame() {
        m_numVerts  = 0;
        m_navUsed   = false;
    }

    UIContext::UIContext() : m_vbo(sf::VertexArray(sf::PrimitiveType::Quads, 16)) {
    }
} // padi