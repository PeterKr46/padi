//
// Created by Peter on 24/04/2022.
//

#pragma once

#include <iostream>

#include "SFML/Graphics/Drawable.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/Transformable.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/System/Time.hpp"

namespace padi {
    class WorldMap
            : public sf::Drawable, public sf::Transformable {

    public:
        bool load(const std::string &tileset, sf::Vector2u tileSize, const int *tiles, unsigned int width,
                  unsigned int height);

        void update(sf::Vector2f & mouse_pos, sf::Time time);

    protected:

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        sf::VertexArray m_vertices;
        sf::Texture m_tileset;
        sf::Vector2u m_size;
        sf::Vector2i m_selected{0,0};
    };

} // padi