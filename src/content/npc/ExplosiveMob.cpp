//
// Created by Peter on 13/06/2022.
//

#include "ExplosiveMob.h"
#include "../../entity/OneshotEntity.h"
#include "../game/OnlineGame.h"
#include "SFML/Network/Packet.hpp"
#include "../../media/AudioPlayback.h"
#include "../vfx/EntityBlink.h"

#include <utility>

namespace padi::content {

    ExplosiveMob::ExplosiveMob(std::string name, const padi::AnimationSet *moveset, const sf::Vector2i &pos)
            : LivingEntity(std::move(name), moveset, pos, EntityType::EXPLOSIVE) {
        setColor(sf::Color(32, 32, 32));
    }

    bool ExplosiveMob::takeTurn(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &chr) {
        auto level = game->getLevel().lock();
        if (!m_turnStarted) {
            bool explode = m_primed;
            if (explode) {
                usedAbility = 1;
                chr->entity->intentCast(chr->abilities[usedAbility], chr->entity->getPosition());
                {
                    CharacterCastPayload payload;
                    payload.ability = uint8_t(1);
                    payload.pos = chr->entity->getPosition(); // ?
                    sf::Packet packet = PackagePayload(payload);
                    printf("[ExplosiveMob] Casting %u at (%i, %i)\n", payload.ability, payload.pos.x, payload.pos.y);
                    game->broadcast(packet);

                    auto rand = std::mt19937(game->getSeed() + chr->id * 45689);
                    auto host = std::static_pointer_cast<HostGame>(game);
                    size_t dir_id = rand() % 4;
                    std::shared_ptr<Tile> tile;
                    size_t placed = 0;
                    for(int i = 0; i < 4 && placed < 3; ++i) {
                        tile = level->getMap()->getTile(chr->entity->getPosition() + AllDirections[(dir_id+i) % 4]);
                        if (tile) {
                            auto tCol = tile->getColor();
                            uint16_t tSum = tCol.r + tCol.g + tCol.b;
                            if(tSum > 96 && tile->m_walkable && !level->getMap()->hasEntities(tile->getPosition(), LIVING)) {
                                auto child1 = std::make_shared<ExplosiveMob>(chr->entity->getName(),
                                                                             chr->entity->getAnimationSet(),
                                                                             tile->getPosition());
                                child1->initHPBar(chr->entity->getHPBar());
                                auto childChr = child1->asCharacter(true);
                                childChr.awake = true;
                                host->spawnCharacter(childChr, ~0u, false);
                                placed++;
                            }
                        }
                    }
                }
                chr->alive = false;
            } else {
                usedAbility = 0;
                auto walk = std::static_pointer_cast<Walk>(chr->abilities[usedAbility]);
                walk->castIndicator(level);
                walk->recalculateRange(level);
                std::vector<sf::Vector2i> targets = walk->getPossibleTargets();
                auto rng = std::mt19937(game->getSeed() + chr->id * 7341);
                std::shuffle(targets.begin(), targets.end(), rng);
                if (targets.empty()) {
                    /*{
                        sf::Packet packet;
                        EntityBlinkPayload payload;
                        payload.cid = chr->id;
                        payload.frequency = 4;
                        PackagePayload(packet, payload);
                        game->broadcast(packet);
                        auto blink = std::make_shared<padi::content::EntityBlink>(shared_from_this(), payload.frequency);
                        level->addFrameBeginListener(blink);
                    }*/
                    m_primed = true;
                    return false;
                }
                auto target = targets.front();
                std::vector<std::shared_ptr<Entity>> ents;
                for (auto pos: targets) {
                    for (auto dir: AllDirections) {
                        auto neighborPos = pos + dir;
                        if (neighborPos != chr->entity->getPosition()) {
                            if (level->getMap()->getEntities(pos + dir, ents)) {
                                for (auto &ent: ents) {
                                    if (ent->getType() & LIVING) {
                                        auto livingEnt = std::static_pointer_cast<LivingEntity>(ent);
                                        if (livingEnt->hasHPBar() && livingEnt->getHPBar().lock()->getHP() > 0) {
                                            target = pos;
                                        }
                                    }
                                }

                            }
                        }
                    }
                }
                chr->entity->intentCast(walk, target);
                {
                    sf::Packet packet;
                    CharacterCastPayload payload;
                    payload.ability = uint8_t(0);
                    payload.pos = target;
                    printf("[ExplosiveMob] Casting %u at (%i, %i)\n", payload.ability, payload.pos.x, payload.pos.y);
                    packet.append(&payload, sizeof(payload));
                    game->broadcast(packet);
                }
            }
            m_turnStarted = true;
        }
        if (chr->entity) {
            if (!chr->entity->hasCastIntent() && chr->abilities[usedAbility]->isCastComplete()) {
                m_turnStarted = false;
                for (auto dir: AllDirections) {
                    if (level->getMap()->hasEntities(chr->entity->getPosition() + dir, LIVING)) {
                        m_primed = true;
                    }
                }
                if(m_primed) {
                    {
                        sf::Packet packet;
                        EntityBlinkPayload payload;
                        payload.cid = chr->id;
                        payload.frequency = 4;
                        PackagePayload(packet, payload);
                        game->broadcast(packet);
                        auto blink = std::make_shared<padi::content::EntityBlink<LivingEntity>>(shared_from_this(), payload.frequency);
                        level->addFrameBeginListener(blink);
                    }
                }
            }
            return !chr->entity->hasCastIntent() && chr->abilities[usedAbility]->isCastComplete();
        } else {
            return true;
        }
    }

    Character ExplosiveMob::asCharacter(bool awake) {
        return {0,
                shared_from_this(),
                {std::make_shared<Walk>(shared_from_this(), 5, Walk::Walkable{-700}),
                 std::make_shared<SelfDestruct>(shared_from_this())},
                [=](const std::shared_ptr<OnlineGame> &l, const std::shared_ptr<Character> &c) {
                    return takeTurn(l, c);
                },
                true,
                awake,
                7
        };
    }


    SelfDestruct::SelfDestruct(std::shared_ptr<padi::LivingEntity> user)
            : Ability(std::move(user)) {

    }

    bool SelfDestruct::cast(const std::weak_ptr<Level> &level, const sf::Vector2i &pos) {
        auto lvl = level.lock();
        m_user->getHPBar().lock()->setHP(0);
        for (auto &direction: Neighborhood) {
            if(lvl->getMap()->getTile(pos + direction)) {
                auto strike = std::make_shared<padi::OneshotEntity>(pos + direction);
                strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
                strike->m_color = sf::Color::Black;
                lvl->addCycleEndListener(strike);
                lvl->getMap()->addEntity(strike);
                std::vector<std::shared_ptr<Entity>> ents;
                if (lvl->getMap()->getEntities(pos + direction, ents)) {
                    for (auto &entity: ents) {
                        if ((entity->getType() & (LIVING | EXPLOSIVE)) == LIVING) {
                            auto livingEntity = std::static_pointer_cast<LivingEntity>(entity);
                            if (livingEntity->hasHPBar()) {
                                auto hpBar = livingEntity->getHPBar().lock();
                                hpBar->setHP(hpBar->getHP() - 1);
                            }
                        }
                    }
                }
            }
        }
        lvl->addFrameBeginListener(shared_from_this());
        auto ap = std::make_shared<padi::AudioPlayback>(lvl->getApollo()->lookupAudio("chord_01"));
        ap->sound.setPitch(0.3);
        ap->setPosition(pos);
        ap->sound.setVolume(100);
        ap->start(lvl);
        return true;
    }

    void SelfDestruct::castCancel(const std::weak_ptr<Level> &level) {

    }

    void SelfDestruct::castIndicator(const std::weak_ptr<Level> &level) {
        // NOP
    }

    bool SelfDestruct::isCastComplete() {
        return m_complete;
    }

    bool SelfDestruct::onFrameBegin(std::weak_ptr<padi::Level> const &level, uint8_t frame) {
        auto lvl = level.lock();
        if (frame < 8) {
            auto pos = m_user->getPosition();
            for (auto &dir: Neighborhood) {
                auto tile = lvl->getMap()->getTile(pos + dir);
                if(tile) {
                    auto col = tile->getColor();
                    uint16_t cSum = col.r + col.g + col.b;
                    if (cSum < 700) {
                        tile->lerpColor(sf::Color(0x1e1e1eff), 0.5);
                        tile->setVerticalOffset(float(frame % 2));
                    }
                }
            }
        } else if (frame == 8) {
            auto pos = m_user->getPosition();
            auto fireAnim = lvl->getApollo()->lookupAnim("fire");
            for (auto &dir: Neighborhood) {
                auto tile = lvl->getMap()->getTile(pos + dir);
                if(tile) {
                    auto col = tile->getColor();
                    uint16_t cSum = col.r + col.g + col.b;
                    if(cSum < 700) {
                        tile->setVerticalOffset(0);
                    }
                    auto fire = std::make_shared<padi::OneshotEntity>(pos + dir);
                    fire->m_animation = fireAnim;
                    fire->m_color = sf::Color::Black;
                    lvl->addCycleEndListener(fire);
                    lvl->getMap()->addEntity(fire, ~0);
                }
            }
            if (m_user->hasHPBar() && m_user->getHPBar().lock()->getHP() == 0) {
                lvl->getMap()->removeEntity(m_user);
            }
        } else if (frame == 11) {
            m_complete = true;
            return false;
        }
        return true;
    }

    uint32_t SelfDestruct::getAbilityType() const {
        return AbilityType::SelfDestruct;
    }
} // content