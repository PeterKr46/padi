//
// Created by Peter on 05/05/2022.
//

#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include "../media/Apollo.h"

namespace padi {

    class UIContext
            : public sf::Drawable {
    private:
        friend class Immediate;

    public:

        UIContext();

        void init(std::string const &apollo, std::string const &sprite);

        const padi::Apollo *getApollo() const;

        bool isFocused(size_t elemID) const;

        void setFocus(size_t elemID);

        size_t numQuads() const;

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        /**
         * Resets the quad counter, etc
         */
        void nextFrame();

        sf::Transform &topTransform();

        sf::Transform popTransform();

        sf::Transform &pushTransform(sf::Transform const &t = sf::Transform());

        void setText(const char* id, std::string const &text, sf::Vector2f const &pos, bool centered = false);
        void updateTextString(const char* id, std::string const &text);
        void updateTextColor(const char* id, sf::Color const& color);

        void removeText(const char* id);

        std::string getText(const char * id);

    private:

        sf::Texture m_sprites;
        padi::Apollo m_apollo;
        sf::VertexArray m_vbo;
        size_t m_numVerts{0};

        struct Text {
            sf::Text text;
            bool centered{};
        };

        sf::Font m_font;
        std::map<size_t, Text> m_text;

    protected:
        std::vector<sf::Transform> m_transformStack{sf::Transform()};
        size_t m_pred{};
        bool m_navUsed{false};
        size_t m_focused{};
    };

} // padi
