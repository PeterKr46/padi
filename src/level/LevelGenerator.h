//
// Created by Peter on 01/05/2022.
//

#pragma once

#include "../../lib/PerlinNoise/PerlinNoise.hpp"
#include <SFML/System/Vector2.hpp>
#include "../map/Map.h"
#include "Level.h"

namespace padi {

    class LevelGenerator {
    public:
        static const uint64_t TutorialSeed = 8008135;

        LevelGenerator& withSpritesheet(std::string const& path);
        LevelGenerator& withApollo(std::string const& path);
        LevelGenerator& withSeed(uint64_t seed);
        LevelGenerator& withArea(sf::Vector2i const& size);
        std::shared_ptr<Level> generate();
        std::shared_ptr<Level> generateTutorial();
    private:
        static void log(std::string const& output);

        uint64_t m_seed;
        siv::PerlinNoise m_perlin;

        std::string m_spritesheetPath;
        std::string m_apolloPath;

        sf::Vector2i m_targetArea{8,8};
    };

} // padi
