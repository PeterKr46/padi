//
// Created by Peter on 24/04/2022.
//

#include "Stage.h"
#include "../lib/PerlinNoise/PerlinNoise.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/Keyboard.hpp"

namespace padi {

    int distanceToCenter(sf::Vector2i const& pos, sf::Vector2i const& center) {
        sf::Vector2i delta = pos - center;
        return std::sqrt(std::pow(delta.x,2) + std::pow(delta.y,2));
    }

    bool Stage::generate(const std::string &tileset, sf::Vector2u tileSize, unsigned int radius) {
        // load the tileset texture
        if (!m_tileset.loadFromFile(tileset))
            return false;

        // resize the vertex array to fit the level size
        m_vertices.setPrimitiveType(sf::PrimitiveType::Quads);
        m_vertices.resize(256);

        sf::Vector2i size = {static_cast<int>(radius*2), static_cast<int>(radius*2)};
        sf::Vector2i center = {static_cast<int>(radius), static_cast<int>(radius)};

        const siv::PerlinNoise::seed_type seed = 123456u;
        const siv::PerlinNoise perlin{ seed };

        m_selector.m_animation = new SimpleAnimation(padi::StripAnimation({32, 32}, {160,0}, {0,32}, 6, 1));

        map.addEntity(&m_selector);
        // populate the vertex array, with one quad per tile
        sf::Vector2i tile{0,0};
        for (tile.x = 0; tile.x < size.x; tile.x++)
            for (tile.y = 0; tile.y < size.y; tile.y++) {
                // get the current tile number (rem.)
                // find its position in the tileset texture
                float z = (perlin.octave2D_01(tile.x * 0.2,tile.y * 0.2, 5));// * (0.9*radius / (1+distanceToCenter(tile, center)));//tiles[i + j * width];
                //if (abs(tile.x - tile.y) < radius ) {
                //    if(z > 0.5)
                auto t = new Tile(tile);
                t->m_color.r = z * 255;
                t->m_color.g = z * 255;
                t->m_color.b = z * 255;
                map.addTile(t);
                //}
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
        auto pos = map.mapWorldPosToTile(getTransform().getInverse().transformPoint(mouse_pos));

        Tile* tile = map.getTile(pos);
        /*if((tile = map.getTile(pos)) == nullptr) {
            tile = new Tile(pos);
            map.addTile(tile);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
            tile->m_color.b = 255;
            //map.populate(m_vertices, 0, {32,32});
        }*/
         if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num0)) tile->m_detail = 0;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) tile->m_detail = 1;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) tile->m_detail = 2;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) tile->m_detail = 3;
        map.moveEntity(&m_selector, pos, ~0u);
        map.populate(m_vertices, 0, {32,32});
    }
} // padi