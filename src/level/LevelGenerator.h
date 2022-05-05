//
// Created by Peter on 01/05/2022.
//

#pragma once

#include "../../lib/PerlinNoise/PerlinNoise.hpp"
#include "SFML/System/Vector2.hpp"
#include "../map/Map.h"
#include "Level.h"

namespace padi {

    class LevelGenerator {
    public:

        LevelGenerator& withSpritesheet(std::string const& path);
        LevelGenerator& withApollo(std::string const& path);
        LevelGenerator& withSeed(uint64_t seed);
        LevelGenerator& withArea(sf::Vector2i const& size);
        Level generate();
    private:
        static void log(std::string const& output);

        siv::PerlinNoise m_perlin;

        std::string m_spritesheetPath;
        std::string m_apolloPath;

        sf::Vector2i m_targetArea{8,8};
        sf::Vector2i m_tileSize{32,32};
    };

} // padi
