//
// Created by Peter on 01/05/2022.
//

#include "LevelGenerator.h"
#include <iostream>
#include <cmath>
#include "../entity/LivingEntity.h"
#include "../entity/StaticEntity.h"
#include "../map/Tile.h"
#include "../Constants.h"
#include "../Utils.h"
#include "../content/vfx/MapShaker.h"

namespace padi {

    LevelGenerator &LevelGenerator::withSeed(uint64_t seed) {
        m_seed = seed;
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


    std::shared_ptr<Level> LevelGenerator::generateLevel() {
        if(m_seed == TutorialSeed) {
            return generateTutorial();
        }

        static const float zScale = 0.2f;
        static const float cScale = 0.05f;
        static const float mScale = 0.05f;

        auto level = std::make_shared<Level>(m_targetArea, padi::TileSize);

        level->m_sprites.loadFromFile(m_spritesheetPath);
        level->m_apollo.loadFromFile(m_apolloPath);

        sf::Vector2i pos;
        sf::Vector2i halfSize = m_targetArea / 2;
        size_t safeRadius = std::min(m_targetArea.x, m_targetArea.y) / 2;
        for (pos.x = -halfSize.x; pos.x < halfSize.x; pos.x++) {
            for (pos.y = -halfSize.y; pos.y < halfSize.y; pos.y++) {
                double dsqr = std::sqrt(pos.x*pos.x + pos.y*pos.y);
                if (dsqr < safeRadius) {
                    auto z = float(m_perlin.octave2D_01(pos.x * zScale, pos.y * zScale, 5)) * (dsqr / safeRadius);
                    auto t = std::make_shared<padi::Tile>(pos);
                    int r = int(16 * (m_perlin.normalizedOctave2D_01(234 + pos.x * cScale, pos.y * cScale, 2))) * 90;
                    float m = (z * m_perlin.normalizedOctave2D_01(pos.x * mScale, pos.y * mScale, 7));
                    t->setColor(sf::Color(hsv(r, 0.2f,  m * 0.5 + 0.5)));
                    //t->setVerticalOffset(int(z * 4));
                    level->getMap()->addTile(t);
                    if (m > 0.25) {
                        auto e = std::make_shared<padi::TileDecoration>(pos);
                        e->m_animation = level->m_apollo.lookupAnim(
                                m > 0.35 ? "peak" : (m > 0.3 ? "mountain" : (m > 0.25 ? "rocks" : "hill")));
                        t->m_walkable = m < 0.25;
                        t->setDecoration(e);
                    }
                    else if(m > 0.12) {
                        auto e = std::make_shared<padi::TileDecoration>(pos);
                        e->m_animation = level->m_apollo.lookupAnim(int(m * 12) % 2 ? "pillars" : "houses_b");
                        t->setDecoration(e);
                    }
                }
            }
        }
        auto center = level->getMap()->getTile(0,0);

        level->centerView(m_targetArea / 2);
        level->initCursor("cursor");
        level->addFrameBeginListener(std::make_shared<padi::content::MapShaker>());
        return level;
    }

    LevelGenerator &LevelGenerator::withApollo(const std::string &path) {
        m_apolloPath = path;
        return *this;
    }

    std::shared_ptr<Level> LevelGenerator::generateTutorial() {
        auto level = std::make_shared<Level>(m_targetArea, padi::TileSize);
        level->m_sprites.loadFromFile(m_spritesheetPath);
        level->m_apollo.loadFromFile(m_apolloPath);

        auto map = level->getMap();
        auto & apollo = level->m_apollo;

        std::shared_ptr<padi::TileDecoration> decor;
        std::shared_ptr<padi::Tile> tile;
        std::mt19937 rand(m_seed);

        auto mountain = apollo.lookupAnim("mountain");
        for(sf::Vector2i pos = {-8, -8}; pos.x <= 8; ++pos.x) {
            for ( pos.y = -8; pos.y <= 8; ++pos.y) {
                tile = std::make_shared<padi::Tile>(pos);
                tile->setColor(sf::Color(0x484848FF));
                float rad = sqrt(float(pos.x * pos.x) + float(pos.y * pos.y));
                if(rad > 7) {
                    decor = std::make_shared<padi::TileDecoration>(pos);
                    decor->m_animation = mountain;
                    tile->m_walkable = false;
                    tile->m_decoration = decor;
                    tile->m_verticalOffset = (rad - 7) * 12;
                }
                map->addTile(tile);
            }
        }

        auto rocks = apollo.lookupAnim("rocks");
        auto hill = apollo.lookupAnim("hill");
        auto pillars = apollo.lookupAnim("pillars");
        for(int i = 0; i < 20; ++i) {
            sf::Vector2i spawn{int(rand() % 14) - 7, int(rand() % 14) - 7};
            decor = std::make_shared<padi::TileDecoration>(spawn);
            if(rand() % 2) {
                decor->m_animation = rocks;
                tile = map->getTile(spawn);
                tile->m_decoration = decor;
                tile->m_walkable = false;
                for (auto &dir: AllDirections) {
                    tile = map->getTile(spawn + dir);
                    if (!tile->m_decoration) {
                        decor = std::make_shared<padi::TileDecoration>(spawn + dir);
                        decor->m_animation = hill;
                        tile->m_decoration = decor;
                    }
                }
            }
            else {
                decor->m_animation = pillars;
                tile = map->getTile(spawn);
                tile->m_decoration = decor;
                tile->m_walkable = false;
            }
        }
        level->addFrameBeginListener(std::make_shared<padi::content::MapShaker>());
        level->centerView({0,0});
        level->initCursor("cursor");
        return level;
    }
} // padi