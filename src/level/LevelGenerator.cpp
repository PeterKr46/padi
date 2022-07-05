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

#include <stack>

namespace padi {

    LevelGenerator &LevelGenerator::withSeed(uint64_t seed) {
        m_seed = seed;
        m_perlin = siv::PerlinNoise{static_cast<siv::PerlinNoise::seed_type>(seed)};
        m_rand = std::mt19937(seed);
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
    struct vecHash
    {
        size_t operator()(const sf::Vector2i &v) const
        {
            return hash_c_string(reinterpret_cast<const char *>(&v), sizeof(sf::Vector2i));
        }
    };

#define MAZE_RESOLUTION 9
#define PATH_WIDTH_MAX 4
#define HEIGHT_NOISE_SCALE 0.3743f

#define COLOR_NOISE_SCALE 0.134512f
#define COLOR_NOISE_OFFSET 945861

#define DECOR_NOISE_SCALE 0.2645
#define DECOR_CUTOFF 0.8
#define DECOR_OFFSET 4562345

    std::shared_ptr<Level> LevelGenerator::generateLevel() {
        if(m_seed == TutorialSeed) {
            return generateTutorial();
        }


        auto level = std::make_shared<Level>(m_targetArea, padi::TileSize);
        auto map = level->getMap();
        auto apollo = level->getApollo();

        level->m_sprites.loadFromFile(m_spritesheetPath);
        level->m_apollo.loadFromFile(m_apolloPath);


        sf::Vector2i gridAnchor;
        std::stack<sf::Vector2i> mazeStack;
        std::unordered_set<sf::Vector2i, vecHash> visited;

        mazeStack.emplace(0,0);
        visited.emplace(0,0);

        auto height= [this] (int x, int y) -> float {
            return float(m_perlin.octave2D_01(float(x) * HEIGHT_NOISE_SCALE, float(y) * HEIGHT_NOISE_SCALE, 5));
        };

        auto weightedHeight = [this, &height] (int x, int y) -> float {
            auto radSquared = x * x + y * y;
            auto maxRadSquared = m_targetArea.x * m_targetArea.x / 4 + m_targetArea.y * m_targetArea.y / 4;
            return height(x,y)
                    * std::min(1.f, float(std::abs(radSquared - maxRadSquared))/5.f);
        };

        auto color = [this] (int x, int y) -> uint32_t {
            int hue = int(720 * m_perlin.octave2D_01(float(x + COLOR_NOISE_OFFSET) * COLOR_NOISE_SCALE, float(y + COLOR_NOISE_OFFSET) * COLOR_NOISE_SCALE, 2));
            return hsv(hue, 0.3f, 0.5f);
        };

        auto mountainGeneration = [this] (int x, int y, float height) -> std::pair<bool, const char*> {

            int dx = (x % MAZE_RESOLUTION) - MAZE_RESOLUTION / 2;
            int dy = (y % MAZE_RESOLUTION) - MAZE_RESOLUTION / 2;
            float h = 1.f - float(dx*dx + dy*dy) / (MAZE_RESOLUTION * 2);
            auto decoNoise = float(m_perlin.octave2D_01(float(x * DECOR_OFFSET) * DECOR_NOISE_SCALE, float(y * DECOR_OFFSET) * DECOR_NOISE_SCALE, 2));
            int mountains = int((h * 16));
            switch (mountains) {
                case 0:
                case 1:
                case 2:
                    return {true, ""};
                case 3:
                case 4:
                case 5:
                    return {true, "hill"};
                case 6:
                case 7:
                    return {false, "rocks"};
                case 8:
                case 9:
                case 10:
                case 11:
                    return {false, "mountain"};
                case 12:
                case 13:
                case 14:
                case 15:
                default:
                    return {false, "peak"};
            }
        };

        auto maxRadSquared = m_targetArea.x * m_targetArea.x / 4 + m_targetArea.y * m_targetArea.y / 4;
        int offsets[4] {0,1,2,3};
        while(!mazeStack.empty()) {
            gridAnchor = mazeStack.top();
            mazeStack.pop();
            std::shuffle(offsets, offsets+4, m_rand);
            auto nextDir = 0;
            for(auto d = 0; d < 4; ++d) {
                auto neighbor = gridAnchor + AllDirections[nextDir + offsets[d]] * MAZE_RESOLUTION;
                if(neighbor.x * neighbor.x + neighbor.y * neighbor.y > maxRadSquared) {
                    continue;
                }
                if (visited.find(neighbor) == visited.end() || m_rand() % 100 > 90) {
                    // GENERATE CONNECTOR
                    for (auto i = 0; i < MAZE_RESOLUTION; ++i) {
                        auto p = gridAnchor + AllDirections[nextDir + offsets[d]] * i;
                        float tHeight = weightedHeight(p.x, p.y);
                        int power = int(1 + std::round(tHeight * (0.5f * PATH_WIDTH_MAX)));
                        auto orthDir = orthAxis(AllDirections[nextDir + offsets[d]]);
                        bool isNew;
                        for(int o = -power; o <= power; ++o) {
                            auto tile = map->addTileIfNone(p + orthDir * o, &isNew);
                            if(isNew) {
                                sf::Color tColor{color(p.x + orthDir.x * o, p.y + orthDir.y * o)};
                                tile->setColor(tColor);
                                if(abs(o) == power && i > 0.2f * MAZE_RESOLUTION && i < 0.8f * MAZE_RESOLUTION && m_rand() % 8 > 4) {
                                    auto decor = std::make_shared<TileDecoration>(p + orthDir * o,
                                                                                  apollo->lookupAnim("rocks"));
                                    tile->setDecoration(decor);
                                    tile->m_walkable = false;
                                } else {
                                    uint8_t structure = m_rand() % UINT8_MAX;
                                    if (structure > 240) {
                                        auto decor = std::make_shared<TileDecoration>(p + orthDir * o,
                                                                                      apollo->lookupAnim("pillars"));
                                        tile->setDecoration(decor);
                                    } else if (structure > 235) {
                                        auto decor = std::make_shared<TileDecoration>(p + orthDir * o,
                                                                                      apollo->lookupAnim("houses_a"));
                                        tile->setDecoration(decor);
                                    } else if (structure > 230) {
                                        auto decor = std::make_shared<TileDecoration>(p + orthDir * o,
                                                                                      apollo->lookupAnim("houses_b"));
                                        tile->setDecoration(decor);
                                    }
                                }
                            }
                        }

                    }
                    visited.insert(neighbor);
                    mazeStack.push(gridAnchor);
                    mazeStack.push(neighbor);
                    d = 5;
                }
            }
        }
        for (int x = -4; x < 4; x++) {
            for (int y = -4; y < 4; y++) {
                if (x * x + y * y > 15) continue;
                map->addTileIfNone({x, y});
            }
        }
        for(auto & cellAnchor : visited) {
            // GENERATE CELL
            sf::Vector2i v;
            for(v.x = -MAZE_RESOLUTION;       v.x < MAZE_RESOLUTION; v.x++ ) {
                for (v.y = -MAZE_RESOLUTION; v.y < MAZE_RESOLUTION; v.y++) {
                    bool isNew;
                    float tHeight = weightedHeight(cellAnchor.x + v.x, cellAnchor.y + v.y);
                    float adjustedHeight = tHeight * std::max(0.f, 1.f - (float(v.x * v.x + v.y * v.y) / (MAZE_RESOLUTION * 2.f)));
                    if (adjustedHeight > 0.2) {
                        auto tile = map->addTileIfNone(cellAnchor + v, &isNew);
                        if(isNew || adjustedHeight < 0.23) {
                            sf::Color tColor{color(cellAnchor.x + v.x, cellAnchor.y + v.y)};
                            tile->setColor(tColor);
                            auto decor = tile->getDecoration();
                            if (!decor) {
                                decor = std::make_shared<TileDecoration>(cellAnchor + v, nullptr);
                                tile->setDecoration(decor);
                            }
                            float mountainHeight = (1 - adjustedHeight);// * std::max(0.f, 0.8f - float(v.x * v.x + v.y * v.y) / (MAZE_RESOLUTION * 2.f));
                            auto mountain = mountainGeneration(cellAnchor.x + v.x, cellAnchor.y + v.y, mountainHeight);
                            if (mountain.second[0] != '\0') {
                                decor->m_animation = apollo->lookupAnim(mountain.second);
                                tile->m_walkable = mountain.first;
                            } else {
                                uint8_t structure = m_rand() % UINT8_MAX;
                                if (structure > 230) {
                                    decor->m_animation = apollo->lookupAnim("pillars");
                                } else {
                                    decor->m_animation = apollo->lookupAnim("pillars");
                                }
                            }
                        }
                    }
                }
            }
        }
        level->centerView({0,0});
        level->initCursor("cursor");
        return level;
    }
} // padi