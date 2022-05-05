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
#include "Map.h"
#include "Entity.h"
#include "LivingEntity.h"
#include "Actions.h"

namespace padi {
    class Stage
            : public sf::Drawable, public sf::Transformable {

    public:
        bool generate(const std::string &tileset, sf::Vector2u tileSize, unsigned int radius);

        void update(sf::Vector2f & mouse_pos, sf::Time time);


        Map* getMap();

        void addFrameListener(FrameListener & func);

    protected:

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        unsigned char mdb_lastFrame{0};

        padi::Map m_map;
        padi::SlaveEntity m_selector{{1,1}};
        sf::VertexArray m_vertices;
        sf::Texture m_tileset;
        std::vector<FrameListener> m_activeTriggers;
    };

} // padi