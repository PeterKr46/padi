//
// Created by Peter on 05/05/2022.
//

#pragma once

#include "SFML/Graphics/Drawable.hpp"
#include "UIObject.h"
#include "SFML/Graphics/Texture.hpp"
#include "../media/Apollo.h"

namespace padi {

    class UIContext
            : public sf::Drawable {

    public:
        void addObject(const std::shared_ptr<padi::UIObject>& obj);
        void removeObject(const std::shared_ptr<padi::UIObject>& obj);

        void populateVBO();

        void initTextures(std::string const& apollo, std::string const& sprite);

        const padi::Apollo* getApollo() const;

    private:
        size_t numQuads() const;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    private:

        sf::Texture m_sprites;
        padi::Apollo m_apollo;
        sf::VertexArray m_vbo;
        size_t m_numVerts{0};

        std::vector<std::shared_ptr<padi::UIObject>> m_objects;
    };

} // padi
