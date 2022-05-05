//
// Created by Peter on 24/04/2022.
//

#include "Stage.h"
#include "../lib/PerlinNoise/PerlinNoise.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/Keyboard.hpp"


namespace padi {
    bool Stage::generate(const std::string &tileset, sf::Vector2u tileSize, unsigned int radius) {
        // load the tileset texture
        if (!m_tileset.loadFromFile(tileset))
            return false;

        // resize the vertex array to fit the level size
        m_vertices.setPrimitiveType(sf::PrimitiveType::Quads);
        m_vertices.resize(256);

        sf::Vector2i size = {static_cast<int>(radius*2), static_cast<int>(radius*2)};

        const siv::PerlinNoise::seed_type seed = 123456u;
        const siv::PerlinNoise perlin{ seed };

        // populate the vertex array, with one quad per tile
        sf::Vector2i tile{0,0};
        for (tile.x = 0; tile.x < size.x; tile.x++)
            for (tile.y = 0; tile.y < size.y; tile.y++) {
                // get the current tile number (rem.)
                // find its position in the tileset texture
                int z = perlin.octave2D_01(tile.x * 0.1,tile.y * 0.1, 3) * radius / std::max(1.f,std::abs(tile.x-float(radius))+std::abs(tile.y-float(radius)));//tiles[i + j * width];
                if (abs(tile.x - tile.y) > radius ) {
                   z = radius*(-3);
                }
                map.addTile(new Tile(tile));
            }
        map.populate(m_vertices, 0, {32,32});
        return true;
    }

    void Stage::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        // apply the transform
        states.transform *= getTransform();

        // apply the tileset texture
        states.texture = &m_tileset;

        // draw the vertex array
        target.draw(m_vertices, states);
    }

    void Stage::update(sf::Vector2f &mouse_pos, sf::Time loop_state) {
        int frame = int(loop_state.asSeconds() * 12) % 12;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
            map.addTile(new Tile(map.mapWorldPosToTile(getTransform().getInverse().transformPoint(mouse_pos))));
            map.populate(m_vertices, 0, {32,32});
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace)) {
            map.addEntity(new Entity(map.mapWorldPosToTile(getTransform().getInverse().transformPoint(mouse_pos))));
            map.populate(m_vertices, 0, {32,32});
        }
    }
} // padi