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
        void clear();

    private:

        sf::Texture m_sprites;
        padi::Apollo m_apollo;
        sf::VertexArray m_vbo;
        size_t m_numVerts{0};

    protected:
        size_t m_pred{};
        bool m_navUsed{false};
        size_t m_focused{};
    };

} // padi
