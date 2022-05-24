//
// Created by Peter on 05/05/2022.
//

#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Texture.hpp>
#include "../media/Apollo.h"
#include "SFML/Graphics/VertexArray.hpp"

namespace padi {

    class UIContext
            : public sf::Drawable {
    private: friend class Immediate;

    public:

        UIContext();

        void init(std::string const& apollo, std::string const& sprite);

        const padi::Apollo* getApollo() const;

        bool isFocused(size_t elemID) const;
        void setFocus(size_t elemID);

        size_t numQuads() const;

        virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        /**
         * Resets the quad counter, etc
         */
        void nextFrame();

        sf::Transform & topTransform();

        sf::Transform popTransform();
        sf::Transform & pushTransform(sf::Transform const& t = sf::Transform());

    private:

        sf::Texture m_sprites;
        padi::Apollo m_apollo;
        sf::VertexArray m_vbo;
        size_t m_numVerts{0};

    protected:
        std::vector<sf::Transform> m_transformStack{sf::Transform()};
        size_t m_pred{};
        bool m_navUsed{false};
        size_t m_focused{};
    };

} // padi
