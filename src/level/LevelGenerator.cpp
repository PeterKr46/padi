//
// Created by Peter on 01/05/2022.
//

#include <iostream>
#include "LevelGenerator.h"
#include "../entity/LivingEntity.h"
#include "../map/Tile.h"

namespace padi {

    LevelGenerator &LevelGenerator::withSeed(uint64_t seed) {
        m_perlin = siv::PerlinNoise{static_cast<siv::PerlinNoise::seed_type>(seed)};
        printf("[LVL] SEED %llu\n", seed);
        return *this;
    }

    LevelGenerator &LevelGenerator::withSpritesheet(const std::string &path) {
        m_spritesheetPath = path;
        return *this;
    }

    void LevelGenerator::log(std::string const &msg) {
        std::cout << "[LevelGen] " << msg << std::endl;
    }

    LevelGenerator &LevelGenerator::withArea(const sf::Vector2i &size) {
        m_targetArea = size;
        return *this;
    }

    sf::Color hsv(int hue, float sat, float val) {
        hue %= 360;
        while (hue < 0) hue += 360;

        if (sat < 0.f) sat = 0.f;
        if (sat > 1.f) sat = 1.f;

        if (val < 0.f) val = 0.f;
        if (val > 1.f) val = 1.f;

        int h = hue / 60;
        float f = float(hue) / 60 - h;
        float p = val * (1.f - sat);
        float q = val * (1.f - sat * f);
        float t = val * (1.f - sat * (1 - f));

        switch (h) {
            default:
            case 0:
            case 6:
                return sf::Color(val * 255, t * 255, p * 255);
            case 1:
                return sf::Color(q * 255, val * 255, p * 255);
            case 2:
                return sf::Color(p * 255, val * 255, t * 255);
            case 3:
                return sf::Color(p * 255, q * 255, val * 255);
            case 4:
                return sf::Color(t * 255, p * 255, val * 255);
            case 5:
                return sf::Color(val * 255, p * 255, q * 255);
        }
    }

    Level LevelGenerator::generate() {
        static const float zScale = 0.2f;
        static const float cScale = 0.05f;
        static const float mScale = 0.05f;

        auto level = std::make_shared<Level>(m_targetArea, m_tileSize);

        level->m_sprites.loadFromFile(m_spritesheetPath);
        level->m_apollo.loadFromFile(m_apolloPath);

        sf::Vector2i pos;
        sf::Vector2i halfSize = m_targetArea / 2;
        size_t safeRadius = std::min(m_targetArea.x, m_targetArea.y) / 2;
        for (pos.x = -halfSize.x; pos.x < halfSize.x; pos.x++) {
            for (pos.y = -halfSize.y; pos.y < halfSize.y; pos.y++) {
                double dsqr = sqrt(pow(pos.x, 2) + pow(pos.y, 2));
                if (dsqr < safeRadius) {
                    auto z = float(m_perlin.octave2D_01(pos.x * zScale, pos.y * zScale, 5)) * (1 - dsqr / safeRadius);
                    auto t = std::make_shared<padi::Tile>(pos);
                    int r = floor(16 * (m_perlin.normalizedOctave2D_01(234 + pos.x * cScale, pos.y * cScale, 2))) * 90;
                    float m = (z * m_perlin.normalizedOctave2D_01(pos.x * mScale, pos.y * mScale, 7));
                    //t->m_detail = std::max(0.,m_perlin.octave2D_01((pos.x - 320) * zScale, pos.y * zScale, 3) * 3.9 - 0.3);

                    /*uint8_t g = 255.f * z * m_perlin.normalizedOctave2D_01(pos.x * cScale, 2345+ pos.y * cScale, 2);
                    uint8_t b = 255.f * z * m_perlin.normalizedOctave2D_01( 768 +pos.x * cScale, 789 +pos.y * cScale, 2);*/
                    t->setColor(hsv(r, 0.3f * cos(z), 0.8));
                    //t->setVerticalOffset(z*4);
                    level->getMap()->addTile(t);
                    if (m > 0.12) {
                        auto e = std::make_shared<padi::StaticEntity>(pos);
                        e->m_animation = level->m_apollo.lookupAnim(m > 0.35 ? "peak" : (m > 0.3 ? "mountain" : (m > 0.25 ? "rocks" : "hill")));
                        t->setVerticalOffset(3);
                        t->m_walkable = m < 0.25;
                        t->setDecoration(e);
                    }
                }
            }
        }
        level->centerView(m_targetArea / 2);
        return *level;
    }

    LevelGenerator &LevelGenerator::withApollo(const std::string &path) {
        m_apolloPath = path;
        return *this;
    }
} // padi