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
            target.draw(v.text, states);
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
        m_transformStack = {sf::Transform()};
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

    size_t hash_c_string(const char *p, size_t s) {
        size_t result = 0;
        const size_t prime = 31;
        for (size_t i = 0; i < s; ++i) {
            result = p[i] + (result * prime);
        }
        return result;
    }

    void UIContext::removeText(const char *id) {
        auto idHash = hash_c_string(id, strlen(id));
        auto found = m_text.find(idHash);
        if (found != m_text.end()) {
            m_text.erase(found);
        }
    }

    void UIContext::setText(const char *id, const std::string &text, const sf::Vector2f &pos, bool centered) {
        auto idHash = hash_c_string(id, strlen(id));
        auto found = m_text.find(idHash);
        if (found != m_text.end()) {
            found->second.text.setString(text);
            found->second.text.setPosition(m_transformStack.back().transformPoint(pos) -
                                           (centered ? (found->second.text.getGlobalBounds().getSize() / 2.f)
                                                     : sf::Vector2f(0, 0)));
        } else {
            Text &t = m_text[idHash];
            t.text = sf::Text(text, m_font, 7);
            t.text.setLineSpacing(1.25);
            t.text.setPosition(topTransform().transformPoint(pos) -
                               (centered ? (t.text.getGlobalBounds().getSize() / 2.f)
                                         : sf::Vector2f(0, 0)));
            t.centered = centered;
        }
    }

    const sf::Text *UIContext::getText(const char *id) const {
        auto idHash = hash_c_string(id, strlen(id));
        auto found = m_text.find(idHash);
        std::string result;
        if (found != m_text.end()) {
            return &found->second.text;
        }
        return nullptr;
    }

    std::string UIContext::getTextString(const char *id) const {
        auto idHash = hash_c_string(id, strlen(id));
        auto found = m_text.find(idHash);
        std::string result;
        if (found != m_text.end()) {
            result = found->second.text.getString();
        }
        return result;
    }

    void UIContext::updateTextString(const char *id, const std::string &str) {
        auto idHash = hash_c_string(id, strlen(id));
        auto found = m_text.find(idHash);
        if (found != m_text.end()) {
            auto &text = found->second.text;
            bool centered = found->second.centered;
            auto p = found->second.text.getPosition();
            if (centered) {
                p += (text.getGlobalBounds().getSize() / 2.f);
            }
            text.setString(str);
            text.setPosition(p - (centered ? (text.getGlobalBounds().getSize() / 2.f) : sf::Vector2f(0, 0)));
        }
    }

    void UIContext::updateTextColor(const char *id, sf::Color const &color) {
        auto idHash = hash_c_string(id, strlen(id));
        auto found = m_text.find(idHash);
        if (found != m_text.end()) {
            found->second.text.setFillColor(color);
        }
    }

    void UIContext::updateTextOutline(const char *id, const sf::Color &color, float thickness) {
        auto idHash = hash_c_string(id, strlen(id));
        auto found = m_text.find(idHash);
        if (found != m_text.end()) {
            found->second.text.setOutlineColor(color);
            found->second.text.setOutlineThickness(thickness);
        }
    }

    void UIContext::updateTextSize(const char *id, float size) {
        auto idHash = hash_c_string(id, strlen(id));
        auto found = m_text.find(idHash);
        if (found != m_text.end()) {
            auto &text = found->second.text;
            bool centered = found->second.centered;
            auto p = text.getPosition();
            if (centered) {
                p += (text.getGlobalBounds().getSize() / 2.f);
            }
            text.setCharacterSize(char(7 * size));
            text.setPosition(p - (centered ? (text.getGlobalBounds().getSize() / 2.f) : sf::Vector2f(0, 0)));
        }
    }
} // padi