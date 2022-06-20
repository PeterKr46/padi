//
// Created by Peter on 13/06/2022.
//

#include "Mob.h"
#include "../../entity/OneshotEntity.h"
#include "../game/OnlineGame.h"
#include "SFML/Network/Packet.hpp"
#include "../../media/AudioPlayback.h"

#include <utility>

namespace padi::content {

    Mob::Mob(std::string name, const padi::AnimationSet *moveset, const sf::Vector2i &pos)
            : LivingEntity(std::move(name), moveset, pos) {
        setColor(sf::Color(64, 64, 64));
    }

    bool Mob::takeTurn(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &chr) {
        auto level = game->getLevel().lock();
        if (!m_turnStarted) {
            bool explode = false;
            for (auto dir: AllDirections) {
                if (level->getMap()->hasEntities(chr->entity->getPosition() + dir)) {
                    explode = true;
                }
            }
            if (explode) {
                chr->entity->intentCast(chr->abilities[1], chr->entity->getPosition());
                {
                    CharacterCastPayload payload;
                    payload.ability = uint8_t(1);
                    payload.pos = level->getCursorLocation();
                    sf::Packet packet = PackagePayload(payload);
                    printf("[Mob] Casting %u at (%i, %i)\n", payload.ability, payload.pos.x, payload.pos.y);
                    game->broadcast(packet);
                }
                chr->alive = false;
            } else {
                auto walk = std::static_pointer_cast<Walk>(chr->abilities[0]);
                walk->castIndicator(level.get());
                auto const &targets = walk->getPossibleTargets();
                if (targets.empty()) {
                    walk->castCancel(level.get());
                    return true; // TODO
                }
                auto target = targets.back();
                for (auto pos: targets) {
                    for (auto dir: AllDirections) {
                        if (pos + dir != chr->entity->getPosition() && level->getMap()->hasEntities(pos + dir)) {
                            target = pos;
                        }
                    }
                }
                chr->entity->intentCast(walk, target);
                {
                    sf::Packet packet;
                    CharacterCastPayload payload;
                    payload.ability = uint8_t(0);
                    payload.pos = target;
                    printf("[Mob] Casting %u at (%i, %i)\n", payload.ability, payload.pos.x, payload.pos.y);
                    packet.append(&payload, sizeof(payload));
                    game->broadcast(packet);
                }
            }
            m_turnStarted = true;
        }
        if (chr->entity) {
            if (!chr->entity->hasCastIntent() && chr->abilities[0]->isCastComplete()) {
                m_turnStarted = false;
            }
            return !chr->entity->hasCastIntent() && chr->abilities[0]->isCastComplete();
        } else {
            return true;
        }
    }

    Character Mob::asCharacter(uint32_t id) {
        return {id,
                shared_from_this(),
                {std::make_shared<Walk>(shared_from_this(), 5), std::make_shared<SelfDestruct>(shared_from_this())},
                [=](const std::shared_ptr<OnlineGame> &l, const std::shared_ptr<Character> &c) {
                    return takeTurn(l, c);
                }
        };
    }


    SelfDestruct::SelfDestruct(std::shared_ptr<padi::LivingEntity> user)
            : Ability(std::move(user)) {

    }

    bool SelfDestruct::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        for(auto & direction : Neighborhood) {
            auto strike = std::make_shared<padi::OneshotEntity>(pos + direction);
            strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
            strike->m_color = sf::Color::Black;
            lvl->addCycleEndListener(strike);
            lvl->getMap()->addEntity(strike);
        }
        lvl->getMap()->getTile(pos)->m_walkable = false;
        lvl->addFrameBeginListener(shared_from_this());
        auto ap = std::make_shared<padi::AudioPlayback>(lvl->getApollo()->lookupAudio("chord_01"));
        ap->sound.setPitch(0.3);
        auto audioPos = Map::mapTilePosToWorld(pos);
        ap->sound.setPosition(audioPos.x, audioPos.y, 0);
        ap->sound.setVolume(100);
        ap->start(lvl);
        return true;
    }

    void SelfDestruct::castCancel(padi::Level *level) {

    }

    void SelfDestruct::castIndicator(padi::Level *level) {
        // NOP
    }

    bool SelfDestruct::isCastComplete() {
        return m_complete;
    }

    bool SelfDestruct::onFrameBegin(padi::Level *lvl, uint8_t frame) {
        if (frame < 8) {
            auto pos = m_user->getPosition();
            for(auto & dir : Neighborhood) {
                auto tile = lvl->getMap()->getTile(pos + dir);
                auto color = tile->getColor();
                color = sf::Color(std::max(48, color.r - 32), std::max(48, color.g - 32), std::max(48, color.b - 32),
                                  255);
                tile->setColor(color);
                tile->setVerticalOffset(float(frame % 2));
            }
        } else if (frame == 8) {
            auto pos = m_user->getPosition();
            for(auto & dir : Neighborhood) {
                auto tile = lvl->getMap()->getTile(pos + dir);
                tile->setVerticalOffset(0);
                tile->m_walkable = false;
                auto fire = std::make_shared<padi::StaticEntity>(pos + dir);
                fire->m_animation = lvl->getApollo()->lookupAnim("fire");
                fire->m_color = sf::Color::Black;
                lvl->getMap()->addEntity(fire,~0);
            }
            lvl->getMap()->removeEntity(m_user);
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