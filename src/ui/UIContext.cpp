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
        for (auto const &[k, v]: m_text) {
            target.draw(v, states);
        }
    }

    size_t UIContext::numQuads() const {
        return m_numVerts / 4;
    }

    void UIContext::init(const std::string &apollo, const std::string &sprite) {
        m_apollo.loadFromFile(apollo);
        m_sprites.loadFromFile(sprite);

        m_font.setSmooth(false);
        m_font.loadFromFile("../media/prstartk.ttf");
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
        m_numVerts = 0;
        m_navUsed = false;
    }

    UIContext::UIContext() : m_vbo(sf::VertexArray(sf::PrimitiveType::Quads, 16)) {
    }

    sf::Transform &UIContext::topTransform() {
        return m_transformStack.back();
    }

    sf::Transform UIContext::popTransform() {
        sf::Transform t;
        if (m_transformStack.empty()) {
            printf("[padi::UIContext] ERROR: Transform stack empty before popping!\n");
        } else {
            t = m_transformStack.back();
            m_transformStack.erase(m_transformStack.begin() + (m_transformStack.size() - 1));
        }
        return t;
    }

    sf::Transform &UIContext::pushTransform(sf::Transform const &t) {
        if (m_transformStack.empty()) {
            printf("[padi::UIContext] ERROR: Transform stack was depleted!\n");
            m_transformStack.emplace_back();
        }
        return m_transformStack.emplace_back(sf::Transform(m_transformStack.back()).combine(t));
        //return m_transformStack.back();
    }

    void UIContext::setText(const std::string &id, const std::string &text) {
        static const auto hash = std::hash<std::string>();
        auto idHash = hash(id);
        auto found = m_text.find(idHash);
        if (found != m_text.end()) {
            found->second.setString(text);
        } else {
            m_text[idHash] = sf::Text(text, m_font, 7);
        }
    }

    void UIContext::removeText(const std::string &id) {
        static const auto hash = std::hash<std::string>();
        auto idHash = hash(id);
        auto found = m_text.find(idHash);
        if (found != m_text.end()) {
            m_text.erase(found);
        }
    }

    void UIContext::setText(const std::string &id, const std::string &text, const sf::Vector2f &pos) {
        static const auto hash = std::hash<std::string>();
        auto idHash = hash(id);
        auto found = m_text.find(idHash);
        if (found != m_text.end()) {
            found->second.setString(text);
            found->second.setPosition(m_transformStack.back().transformPoint(pos));
        } else {
            auto txt = sf::Text(text, m_font, 7);
            txt.setLineSpacing(1.25);
            txt.setPosition(m_transformStack.back().transformPoint(pos));
            m_text[idHash] = txt;
        }
    }
} // padi