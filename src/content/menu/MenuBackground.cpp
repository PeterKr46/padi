//
// Created by Peter on 06/05/2022.
//

#include "MenuBackground.h"
#include "../../level/LevelGenerator.h"
#include "../../map/Tile.h"
#include "../../entity/LivingEntity.h"
#include "../../level/SpawnEvent.h"

namespace padi::content {

    class Disolver : public padi::CycleListener, public std::enable_shared_from_this<Disolver> {
    public:
        bool onFrameBegin(Level *level, uint8_t frame) override {
            std::vector<sf::Vector2i> remove;
            auto rm = &remove;
            level->getMap()->for_each([level, rm](auto &&PH1) {
                return Disolver::dissolve(level, rm,
                                          std::forward<decltype(PH1)>(PH1));
            });
            for (auto p: remove) {
                level->getMap()->removeTile(p);
            }
            return true;
        }

        static void dissolve(Level *lvl, std::vector<sf::Vector2i> *remove, std::shared_ptr<Tile> const &tile) {
            static const sf::Vector2i directions[4]{
                    sf::Vector2i(0, 1),
                    sf::Vector2i(0, -1),
                    sf::Vector2i(1, 0),
                    sf::Vector2i(-1, 0)
            };
            auto pos = tile->getPosition();
            auto num = std::count_if(&directions[0], &directions[3],
                                     [lvl, pos](auto &d) { return lvl->getMap()->getTile(pos + d); });
            if (num < 3) {
                tile->setColor(tile->getColor() - sf::Color(8, 8, 8, 0));
                if (tile->getColor() == sf::Color::Black) {
                    if (!lvl->getMap()->hasEntities(pos)) {
                        //lvl->getMap()->removeTile(pos);
                        remove->push_back(tile->getPosition());
                    }
                }
            }
        }
    };

    class RandomizedMovement : public padi::CycleListener, public std::enable_shared_from_this<RandomizedMovement> {
    public:
        explicit RandomizedMovement(std::vector<std::shared_ptr<padi::LivingEntity>> const &entity) {
            m_entities = entity;
        }

        bool onCycleEnd(Level *lvl) override {
            static const sf::Vector2i directions[4]{
                    sf::Vector2i(0, 1),
                    sf::Vector2i(0, -1),
                    sf::Vector2i(1, 0),
                    sf::Vector2i(-1, 0)
            };
            for (auto &entity: m_entities) {
                size_t dir = rand() % 4;
                auto position = entity->getPosition();
                if (entity->isMoving()) {
                    position += entity->currentMoveDirection();
                }
                auto tile = lvl->getMap()->getTile(position + directions[dir]);
                if (tile && tile->m_walkable) {
                    entity->intentMove(directions[dir]);
                    tile->setColor(tile->getColor() + entity->getColor());
                }
                //if (rand() % 3 == 0)lvl->centerView(position);
            }
            return true;
        }

    private:
        std::vector<std::shared_ptr<padi::LivingEntity>> m_entities;
    };

    MenuBackground::MenuBackground() {
        auto levelGen = padi::LevelGenerator();
        // Seed with a real random value, if available
        std::random_device r;

        m_level = levelGen
                .withSpritesheet("../media/level_sheet.png")
                .withApollo("../media/level.apollo")
                .withSeed(6774586)
                .withArea({20, 20})
                .generate();
        std::vector<std::shared_ptr<LivingEntity>> cubes = {
                std::make_shared<padi::LivingEntity>("r", m_level->getApollo()->lookupAnimContext("cube"),
                                                     sf::Vector2i{-2, -5}),
                std::make_shared<padi::LivingEntity>("g", m_level->getApollo()->lookupAnimContext("cube"),
                                                     sf::Vector2i{-5, -2}),
                std::make_shared<padi::LivingEntity>("b", m_level->getApollo()->lookupAnimContext("cube"),
                                                     sf::Vector2i{3, 4})
        };
        m_level->addFrameBeginListener(std::make_shared<Disolver>());
        cubes[0]->setColor(sf::Color::Red);
        cubes[1]->setColor(sf::Color::Green);
        cubes[2]->setColor(sf::Color::Blue);
        for (auto const &cube: cubes) {
            auto leSpawn = std::make_shared<padi::SpawnEvent>(cube);
            leSpawn->dispatch(m_level);
        }
        m_level->addCycleEndListener(std::make_shared<RandomizedMovement>(cubes));
    }

    void MenuBackground::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        m_level->draw(target, states);
    }

    std::shared_ptr<Level> MenuBackground::getLevel() {
        return m_level;
    }
}