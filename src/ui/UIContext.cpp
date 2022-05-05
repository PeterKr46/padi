//
// Created by Peter on 05/05/2022.
//

#include "UIContext.h"

#include <SFML/Graphics.hpp>

namespace padi {
    void UIContext::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        states.texture = &m_sprites;

        states.transform.translate(target.getView().getCenter() - target.getView().getSize() / 2.f);

        // draw the vertex array
        target.draw(&m_vbo[0], m_numVerts, sf::PrimitiveType::Quads, states);

    }

    void UIContext::populateVBO() {
        size_t quads = numQuads();

        if(m_vbo.getVertexCount() <= quads * 4) {
            m_vbo.resize(quads * 6); // double to prevent minor increases causing trouble..?
        }

        m_numVerts = 0;
        for(auto const& object : m_objects) {
            m_numVerts += object->populate(this, m_vbo, m_numVerts, 0);
        }
    }

    size_t UIContext::numQuads() const {
        size_t numVerts = 0;
        for(auto const& object : m_objects) {
            numVerts += object->numQuads();
        }
        return numVerts;
    }

    void UIContext::addObject(const std::shared_ptr<padi::UIObject>& obj) {
        m_objects.push_back(obj);
    }

    void UIContext::removeObject(const std::shared_ptr<padi::UIObject> &obj) {
        auto iter = std::find(m_objects.begin(), m_objects.end(), obj);
        if(iter != m_objects.end()) {
            m_objects.erase(iter);
        }
    }

    void UIContext::initTextures(const std::string &apollo, const std::string &sprite) {
        m_apollo.loadFromFile(apollo);
        m_sprites.loadFromFile(sprite);
    }

    const padi::Apollo *UIContext::getApollo() const {
        return &m_apollo;
    }
} // padi