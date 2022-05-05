//
// Created by Peter on 01/05/2022.
//

#include <iostream>
#include "LevelGenerator.h"

namespace padi {

    LevelGenerator& LevelGenerator::withSeed(uint64_t seed) {
        m_perlin = siv::PerlinNoise {static_cast<siv::PerlinNoise::seed_type>(seed)};
        return *this;
    }

    LevelGenerator& LevelGenerator::loadSpriteMaster(const std::string &path) {
        if(!m_spriteMaster.loadFromFile(path)) {
            log("Failed to load sprite master '" + path + "'!");
        }
        return *this;
    }

    void LevelGenerator::log(std::string const& msg) {
        std::cout << "[LevelGen] " << msg << std::endl;
    }

    LevelGenerator& LevelGenerator::withArea(const sf::Vector2i &size) {
        m_targetArea = size;
        return *this;
    }

    Level LevelGenerator::generate() {
        auto level = std::make_shared<Level>(m_targetArea, m_tileSize);
        level->setMasterSheet(m_spriteMaster);
        sf::Vector2i pos;
        sf::Vector2i center = m_targetArea / 2;
        size_t safeRadius = std::min(m_targetArea.x, m_targetArea.y) +1;
        for (pos.x = 0; pos.x < m_targetArea.x; pos.x++) {
            for (pos.y = 0; pos.y < m_targetArea.y; pos.y++) {
                if (abs(pos.x - center.x) + abs(pos.y - center.y) < safeRadius) {
                    auto z = float(m_perlin.octave2D_01(pos.x * 0.2, pos.y * 0.2, 5));
                    auto t = std::make_shared<padi::Tile>(pos);
                    t->m_color.r = z * 255;
                    t->m_color.g = z * 255;
                    t->m_color.b = z * 255;
                    level->getMap()->addTile(t);
                }
            }
        }
        level->centerView(m_targetArea / 2);
        return *level;
    }
} // padi