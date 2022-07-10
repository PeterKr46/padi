//
// Created by Peter on 08/07/2022.
//

#include "Thirdman.h"

#include <utility>
#include "../game/OnlineGame.h"
#include "../vfx/EntityBlink.h"

namespace padi::content {

    Thirdman::Thirdman(std::string const &name, const padi::AnimationSet *moveset, const sf::Vector2i &pos)
            : LivingEntity(name, moveset, pos) {
        setColor(sf::Color(32, 32, 32));
    }

    bool Thirdman::takeTurn(const std::shared_ptr<OnlineGame> &g, const std::shared_ptr<Character> &chr) {
        auto game = std::static_pointer_cast<HostGame>(g);
        auto level = game->getLevel().lock();
        if (!m_turnStarted) {
            auto map = level->getMap();
            if (!m_rand) {
                m_rand = std::make_shared<std::mt19937>(game->getSeed() + chr->id * 4567);
            }

            sf::Vector2i gridSize = level->getGridSize();

            sf::Vector2i target{0, 0};
            int levelSize = 0;
            while (map->getTile(gridSize.x * levelSize++, 0)) {

            }
            do {
                target.x = int((*m_rand)()) % (2 * levelSize) - levelSize;
                target.y = int((*m_rand)()) % (2 * levelSize) - levelSize;
            } while (!map->getTile(target));
            printf("[Thirdman] Striking %i, %i\n", target.x, target.y);
            //target += sf::Vector2i{int((*m_rand)()) % (gridSize.x / 5), int((*m_rand)()) % (gridSize.y / 5)};

            chr->entity->intentCast(chr->abilities[m_lastAbility], target);
            {
                sf::Packet packet;
                CharacterCastPayload payload;
                payload.ability = uint8_t(m_lastAbility);
                payload.pos = target;
                printf("[Thirdman] Casting %u at (%i, %i)\n", payload.ability, payload.pos.x, payload.pos.y);
                packet.append(&payload, sizeof(payload));
                game->broadcast(packet);
            }
            {
                auto host = std::static_pointer_cast<HostGame>(game);
                sf::Vector2i delta{0, 0};
                int radius = int(gridSize.x / 3.5);
                std::shared_ptr<Tile> tile;
                auto downPour = std::make_shared<EruptionTarget>("target",
                                                                 level->getApollo()->lookupAnimContext("invisible"),
                                                                 target);
                downPour->radius = radius;
                auto downPourChr = downPour->asCharacter();
                downPourChr.awake = true;
                host->spawnCharacter(downPourChr, ~0u, false);
            }
            m_turnStarted = true;
        } else {
            bool result = false;
            if (!chr->entity->hasCastIntent() && chr->abilities[m_lastAbility]->isCastComplete()) {
                m_turnStarted = false;
                m_lastAbility = (m_lastAbility + 1) % 2;
                result = true;
            }
            return result;
        }
        return false;
    }

    Character Thirdman::asCharacter(bool awake) {
        return Character{0,
                         shared_from_this(),
                         {
                                 std::make_shared<Eruption>(shared_from_this()),
                                 std::make_shared<Idle>(shared_from_this())
                         },
                         [=](const std::shared_ptr<OnlineGame> &l, const std::shared_ptr<Character> &c) {
                             return takeTurn(l, c);
                         },
                         true,
                         awake,
                         0
        };
    }

    Eruption::Eruption(std::shared_ptr<LivingEntity> user)
            : Ability(std::move(user)) {

    }

    bool Eruption::cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) {
        auto level = lvl.lock();
        level->addFrameBeginListener(shared_from_this());
        level->addCycleEndListener(shared_from_this());
        m_complete = false;

        auto ose = std::make_shared<padi::OneshotEntityStack>(m_user->getPosition());
        ose->m_animation = level->getApollo()->lookupAnim("lightning");
        ose->m_stackSize = 16;
        ose->m_color = m_user->getColor();
        ose->setVerticalOffset(-32);
        ose->dispatchImmediate(level);
        m_strikePos = pos;
        m_user->trySetAnimation("erupt");
        level->centerView(pos);
        return true;
    }

    void Eruption::castCancel(const std::weak_ptr<Level> &level) {

    }

    void Eruption::castIndicator(const std::weak_ptr<Level> &level) {

    }

    bool Eruption::isCastComplete() {
        return m_complete;
    }

    bool Eruption::onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) {
        m_complete = frame == 8;
        auto level = lvl.lock();
        auto map = level->getMap();
        auto tile = map->getTile(m_user->getPosition());
        tile->setVerticalOffset(float(frame % 2));
        return !m_complete;
    }

    uint32_t Eruption::getAbilityType() const {
        return AbilityType::Eruption;
    }

    bool Eruption::onCycleEnd(const std::weak_ptr<padi::Level> &lvl) {
        sf::Vector2i delta;
        auto level = lvl.lock();
        auto map = level->getMap();
        int radius = int(level->getGridSize().x / 3.5);
        int radSqr = radius * radius;
        std::shared_ptr<Tile> tile;
        auto sprite = level->getApollo()->lookupAnim("indicator");
        for (delta.x = -radius; delta.x <= radius; delta.x++) {
            for (delta.y = -radius; delta.y <= radius; delta.y++) {
                if (radSqr >= delta.x * delta.x + delta.y * delta.y) {
                    tile = map->getTile(m_strikePos + delta);
                    if (tile) {
                        auto marker = std::make_shared<StaticEntity>(m_strikePos + delta, INCOMING);
                        marker->m_animation = sprite;
                        marker->m_color = sf::Color(32, 32, 32);
                        level->addFrameBeginListener(std::make_shared<EntityBlink<StaticEntity>>(marker, 3));
                        map->addEntity(marker);
                    }
                }
            }
        }
        return false;
    }

    EruptionTarget::EruptionTarget(const std::string &name, const AnimationSet *moveset, const sf::Vector2i &pos)
            : LivingEntity(name, moveset, pos) {
        setColor(sf::Color(32,32,32));
    }

    bool EruptionTarget::takeTurn(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &chr) {
        if (!m_turnStarted) {
            chr->entity->intentCast(chr->abilities[0], getPosition());
            {
                sf::Packet packet;
                CharacterCastPayload payload;
                payload.ability = uint8_t(0);
                payload.pos = getPosition();
                printf("[EruptionTarget|%i] Casting %u at (%i, %i)\n", chr->id, payload.ability, payload.pos.x, payload.pos.y);
                packet.append(&payload, sizeof(payload));
                game->broadcast(packet);
            }
            m_turnStarted = true;
            chr->alive = false;
        } else {
            if (!chr->entity->hasCastIntent() && chr->abilities[0]->isCastComplete()) {
                m_turnStarted = false;
            }
            return !chr->entity->hasCastIntent() && chr->abilities[0]->isCastComplete();
        }
        return false;
    }

    Character EruptionTarget::asCharacter() {
        return Character{0,
                         shared_from_this(),
                         {
                                 std::make_shared<DownPour>(shared_from_this(), radius),
                         },
                         [=](const std::shared_ptr<OnlineGame> &l, const std::shared_ptr<Character> &c) {
                             return takeTurn(l, c);
                         },
                         true,
                         true,
                         0
        };
    }

    DownPour::DownPour(std::shared_ptr<LivingEntity> user, int radius)
            : Ability(std::move(user)), m_radius(radius) {

    }

    bool DownPour::cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) {
        sf::Vector2i delta;
        auto level = lvl.lock();
        auto map = level->getMap();
        std::shared_ptr<Tile> tile;
        auto radSqr = m_radius * m_radius;
        auto sprite = level->getApollo()->lookupAnim("air_strike_large");
        for (delta.x = -m_radius; delta.x <= m_radius; delta.x++) {
            for (delta.y = -m_radius; delta.y <= m_radius; delta.y++) {
                if (radSqr >= delta.x * delta.x + delta.y * delta.y) {
                    tile = map->getTile(pos + delta);
                    if (tile) {
                        auto marker = map->getEntity(pos + delta, INCOMING);
                        if (marker) {
                            auto explosion = std::make_shared<OneshotEntity>(pos + delta);
                            explosion->m_color = sf::Color(32, 32, 32);
                            explosion->m_animation = sprite;
                            explosion->dispatchImmediate(level);
                        }
                    }
                }
            }
        }
        map->removeEntity(m_user);
        level->addCycleEndListener(shared_from_this());
        level->addFrameBeginListener(shared_from_this());
        level->centerView(pos);
        m_complete = false;
        m_strikePos = pos;
        return true;
    }

    void DownPour::castCancel(const std::weak_ptr<Level> &level) {

    }

    void DownPour::castIndicator(const std::weak_ptr<Level> &level) {

    }

    bool DownPour::isCastComplete() {
        return m_complete;
    }

    uint32_t DownPour::getAbilityType() const {
        return AbilityType::DownPour;
    }

    bool DownPour::onCycleEnd(const std::weak_ptr<padi::Level> &lvl) {
        m_complete = true;
        sf::Vector2i delta;
        auto level = lvl.lock();
        auto map = level->getMap();
        auto radSqr = m_radius * m_radius;
        std::shared_ptr<Tile> tile;
        auto sprite = level->getApollo()->lookupAnim("air_strike_large");
        for (delta.x = -m_radius; delta.x <= m_radius; delta.x++) {
            for (delta.y = -m_radius; delta.y <= m_radius; delta.y++) {
                if (radSqr >= delta.x * delta.x + delta.y * delta.y) {
                    tile = map->getTile(m_strikePos + delta);
                    if (tile) {
                        auto marker = map->getEntity(m_strikePos + delta, INCOMING);
                        if (marker) {
                            map->removeEntity(marker);
                            tile->setVerticalOffset(0);
                        }
                    }
                }
            }
        }
        return CycleListener::onCycleEnd(lvl);
    }

    bool DownPour::onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) {
        sf::Vector2i delta;
        auto level = lvl.lock();
        auto map = level->getMap();
        std::vector<std::shared_ptr<Entity>> ents;
        std::shared_ptr<Tile> tile;
        for (delta.x = -m_radius; delta.x <= m_radius; delta.x++) {
            for (delta.y = -m_radius; delta.y <= m_radius; delta.y++) {
                tile = map->getTile(m_strikePos + delta);
                if (tile) {
                    auto marker = map->getEntity(m_strikePos + delta, INCOMING);
                    if (marker) {
                        tile->setVerticalOffset(frame % 3);
                        auto col = tile->getColor();
                        uint16_t cSum = col.r + col.g + col.b;
                        if (cSum < 700) {
                            tile->lerpColor(m_user->getColor(), 0.2);
                        }
                        if (frame == 8) {
                            if (map->getEntities(m_strikePos + delta, ents)) {
                                for (auto &entity: ents) {
                                    if (entity->getType() & LIVING) {
                                        auto livingEntity = std::static_pointer_cast<LivingEntity>(entity);
                                        if (livingEntity->hasHPBar() && livingEntity->isLight()) {
                                            auto hpBar = livingEntity->getHPBar().lock();
                                            auto hp = hpBar->getHP();
                                            hpBar->setHP(hp - 1);
                                            if (hp == 1) m_user->enemiesSlain++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return frame < 11;
    }
} // content