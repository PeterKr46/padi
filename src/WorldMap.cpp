//
// Created by Peter on 24/04/2022.
//

#include "WorldMap.h"
#include "../lib/PerlinNoise/PerlinNoise.hpp"


namespace padi {
    bool WorldMap::load(const std::string &tileset, sf::Vector2u tileSize, const int *tiles, unsigned int width,
                        unsigned int height) {
        // load the tileset texture
        if (!m_tileset.loadFromFile(tileset))
            return false;

        // resize the vertex array to fit the level size
        m_vertices.setPrimitiveType(sf::PrimitiveType::Quads);
        m_vertices.resize(width * height * 4);
        m_size = {width,height};

        const siv::PerlinNoise::seed_type seed = 123456u;
        const siv::PerlinNoise perlin{ seed };

        sf::Vector2f anchor;
        int tu;
        int tv;
        // populate the vertex array, with one quad per tile
        for (int i = 0; i < width; ++i)
            for (int j = 0; j < height; ++j) {
                // get the current tile number (rem.)
                // find its position in the tileset texture
                tu = /*tiles[i + j * width]*/(1) % (m_tileset.getSize().x / tileSize.x);
                tv = /*tiles[i + j * width]*/ (1) / (m_tileset.getSize().x / tileSize.x);
                int z = std::min(0.5, perlin.octave2D_01(i * 0.25,j * 0.25, 3) ) * 3;//tiles[i + j * width];
                // get a pointer to the current tile's quad
                sf::Vertex *quad = &m_vertices[(i + j * width) * 4];

                //anchor = sf::Vector2f(((j % 2 == 1) ? tileSize.x * 0.5f : 0), 0);
                //anchor += sf::Vector2f(tileSize.x *i, tileSize.y*0.25 * j);
                anchor = sf::Vector2f ((i-j)*0.5*tileSize.x, (i+j-z)*0.25*tileSize.y);

                // define its 4 corners
                quad[0].position = anchor;
                quad[1].position = anchor+sf::Vector2f(32,0);
                quad[2].position = anchor+sf::Vector2f(32,32);
                quad[3].position = anchor+sf::Vector2f(0,32);

                // define its 4 texture coordinates
                //quad[0].texCoords = sf::Vector2f(tu * tileSize.x, tv * tileSize.y);
                //quad[1].texCoords = sf::Vector2f((tu + 1) * tileSize.x, tv * tileSize.y);
                //quad[2].texCoords = sf::Vector2f((tu + 1) * tileSize.x, (tv + 1) * tileSize.y);
                //quad[3].texCoords = sf::Vector2f(tu * tileSize.x, (tv + 1) * tileSize.y);
                quad[0].texCoords = sf::Vector2f(0, 0);
                quad[1].texCoords = sf::Vector2f(32,0);
                quad[2].texCoords = sf::Vector2f(32,32);
                quad[3].texCoords = sf::Vector2f(0, 32);
            }

        return true;
    }

    sf::Vector2i worldToGrid(sf::Vector2f world) {

        return {int(2*world.y+(world.x-16)) / 32, int(2*world.y-(world.x-16)) / 32 };
    }

    void WorldMap::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        // apply the transform
        states.transform *= getTransform();

        // apply the tileset texture
        states.texture = &m_tileset;

        // draw the vertex array
        target.draw(m_vertices, states);
    }

    void WorldMap::update(sf::Vector2f &mouse_pos, sf::Time loop_state) {
        int frame = int(loop_state.asSeconds() * 12) % 12;
        sf::Vertex *quad;
        if (m_selected.x >= 0 && m_selected.x < m_size.x
            && m_selected.y >= 0 && m_selected.y < m_size.y) {
            quad = &m_vertices[(m_selected.x + m_selected.y * m_size.x) * 4];
            quad[0].texCoords = sf::Vector2f(0, 0 );
            quad[1].texCoords = sf::Vector2f(32,0 );
            quad[2].texCoords = sf::Vector2f(32,32);
            quad[3].texCoords = sf::Vector2f(0, 32);
        }

        m_selected = worldToGrid(getTransform().getInverse().transformPoint(mouse_pos));

        if (m_selected.x >= 0 && m_selected.x < m_size.x
            && m_selected.y >= 0 && m_selected.y < m_size.y) {

            quad = &m_vertices[(m_selected.x + m_selected.y * m_size.x) * 4];
            quad[0].texCoords = sf::Vector2f(0, 0  + frame * 32);
            quad[1].texCoords = sf::Vector2f(32,0  + frame * 32);
            quad[2].texCoords = sf::Vector2f(32,32 + frame * 32);
            quad[3].texCoords = sf::Vector2f(0, 32 + frame * 32);
        }
    }
} // padi