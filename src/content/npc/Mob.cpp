//
// Created by Peter on 13/06/2022.
//

#include "Mob.h"
#include "../../entity/OneshotEntity.h"
#include "../../net/Packets.h"
#include "../game/OnlineGame.h"
#include "SFML/Network/Packet.hpp"

#include <utility>

namespace padi::content {

    Mob::Mob(std::string name, const padi::AnimationSet *moveset, const sf::Vector2i &pos,
             std::vector<Inbox> &sockets)
            : LivingEntity(std::move(name), moveset, pos) {
        m_sockets = sockets;
        setColor(sf::Color(64, 64, 64));
    }

    bool Mob::takeTurn(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &chr) {
        auto level = game->getLevel().lock();
        if (!m_walk) {
            m_walk = std::make_shared<padi::content::Walk>(shared_from_this(), 5);
            m_explode = std::make_shared<padi::content::SelfDestruct>(shared_from_this());
        }
        if (!m_turnStarted) {
            bool explode = false;
            for (auto dir: AllDirections) {
                if (level->getMap()->hasEntities(chr->entity->getPosition() + dir)) {
                    explode = true;
                }
            }
            if (explode) {
                chr->entity->intentCast(m_explode, chr->entity->getPosition());
                {
                    PlayerCastPayload payload;
                    payload.ability = uint8_t(1);
                    payload.pos = level->getCursorLocation();
                    sf::Packet packet = PackagePayload(payload);
                    printf("[Mob] Casting %u at (%i, %i)\n", payload.ability, payload.pos.x, payload.pos.y);
                    for (auto &socket: m_sockets) {
                        socket.getSocket().lock()->send(packet);
                    }
                }
                Corruption corruption{chr->entity->getPosition()};
                chr->controller = [corruption](const std::shared_ptr<OnlineGame> &level,
                                               const std::shared_ptr<Character> &chr) mutable {
                    return corruption.expand(level, chr);
                };
                chr->entity.reset();
            } else {
                m_walk->castIndicator(level.get());
                auto const &targets = m_walk->getPossibleTargets();
                if (targets.empty()) {
                    m_walk->castCancel(level.get());
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
                chr->entity->intentCast(m_walk, target);
                {
                    sf::Packet packet;
                    PlayerCastPayload payload;
                    payload.ability = uint8_t(0);
                    payload.pos = level->getCursorLocation();
                    printf("[Mob] Casting %u at (%i, %i)\n", payload.ability, payload.pos.x, payload.pos.y);
                    packet.append(&payload, sizeof(payload));
                    for (auto &socket: m_sockets) {
                        socket.getSocket().lock()->send(packet);
                    }
                }
            }
            m_turnStarted = true;
        }
        if (chr->entity) {
            if (!chr->entity->hasCastIntent() && m_walk->isCastComplete()) {
                m_turnStarted = false;
            }
            return !chr->entity->hasCastIntent() && m_walk->isCastComplete();
        } else {
            return true;
        }
    }

    Character Mob::asCharacter(uint32_t id) {
        return {id,
                shared_from_this(),
                {m_walk, m_explode},
                [=](const std::shared_ptr<OnlineGame> &l, const std::shared_ptr<Character> &c) {
                    return takeTurn(l, c);
                }
        };
    }

    SelfDestruct::SelfDestruct(std::shared_ptr<padi::LivingEntity> user)
            : Ability(std::move(user)) {

    }

    bool SelfDestruct::cast(padi::Level *lvl, const sf::Vector2i &pos) {
        auto strike = std::make_shared<padi::OneshotEntity>(pos);
        strike->m_animation = lvl->getApollo()->lookupAnim("air_strike_large");
        strike->m_color = sf::Color::Black;
        lvl->addCycleEndListener(strike);
        lvl->getMap()->addEntity(strike);
        lvl->getMap()->getTile(pos)->m_walkable = false;
        lvl->addFrameBeginListener(shared_from_this());
        auto ap = std::make_shared<padi::AudioPlayback>(lvl->getApollo()->lookupAudio("chord_01"));
        ap->sound.setPitch(0.3);
        auto audioPos = Map::mapTilePosToWorld(pos);
        ap->sound.setPosition(audioPos.x, audioPos.y, 0);
        ap->sound.setVolume(100);
        lvl->addCycleEndListener(ap);
        return false;
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
        auto tile = lvl->getMap()->getTile(m_user->getPosition());
        if (frame < 8) {
            auto color = tile->getColor();
            color = sf::Color(std::max(48, color.r - 32), std::max(48, color.g - 32), std::max(48, color.b - 32), 255);
            tile->setColor(color);
            tile->setVerticalOffset(frame % 2);
        } else if (frame == 8) {
            tile->setVerticalOffset(0);
            auto fire = std::make_shared<padi::StaticEntity>(m_user->getPosition());
            fire->m_animation = lvl->getApollo()->lookupAnim("fire");
            fire->m_color = sf::Color::Black;
            lvl->getMap()->addEntity(fire);
            lvl->getMap()->removeEntity(m_user);
        } else if (frame == 11) {
            m_complete = true;
            return false;
        }
        return true;
    }

    Corruption::Corruption(const sf::Vector2i &origin, float power, float decay)
            : m_power(power),
              m_decay(decay) {
        for (auto dir: AllDirections) {
            m_positions.push_back(origin + dir);
        }
    }

    bool Corruption::expand(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &c) {
        auto l = game->getLevel().lock();
        l->centerView(m_positions.front());
        size_t numPos = m_positions.size();
        for (size_t i = 0; i < numPos; ++i) {
            auto tile = l->getMap()->getTile(m_positions[i]);
            tile->setColor(tile->getColor() - sf::Color(255 * m_power, 255 * m_power, 255 * m_power, 0));
            for (auto dir: AllDirections) {
                m_positions.push_back(m_positions[i] + dir);
            }
        }
        m_power *= m_decay;
        if (m_power < 0.05) {
            c->alive = false;
        }
        return true;
    }
} // content