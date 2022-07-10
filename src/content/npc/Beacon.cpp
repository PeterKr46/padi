//
// Created by Peter on 22/06/2022.
//

#include "Beacon.h"
#include "../game/OnlineGame.h"
#include "../../media/AudioPlayback.h"

#include <utility>

namespace padi::content {
    Beacon::Beacon(std::string name, const padi::AnimationSet *moveset, const sf::Vector2i &pos) : LivingEntity(
            std::move(name), moveset, pos, BEACON) {
        setColor(sf::Color(0xffffffff));
        setVerticalOffset(-8);
    }

    bool Beacon::takeTurn(const std::shared_ptr<OnlineGame> &g, const std::shared_ptr<Character> &chr) {
        if(!m_started) {
            auto game = std::static_pointer_cast<HostGame>(g);
            auto level = game->getLevel().lock();

            sf::Vector2i target = chr->entity->getPosition();
            if (!m_open) {
                size_t slain = countSlainEnemies(g);

                if (slain < m_requiredKills) {
                    game->sendChatGeneric("Clear " + std::to_string(m_requiredKills - slain) + " enemies!");
                    target.x++;
                } else {
                    m_open = true;
                    game->sendChatGeneric("THE GATE IS OPEN!");
                    game->sendChatGeneric("Follow the light.");
                }
            } else {
                target.x++;
            }

            chr->entity->intentCast(chr->abilities[0], target);
            {
                sf::Packet packet;
                CharacterCastPayload payload;
                payload.ability = uint8_t(0);
                payload.pos = target;
                packet.append(&payload, sizeof(payload));
                game->broadcast(packet);
            }
            if (m_open) {
                m_safe = true;
                for (auto &[id, c]: game->getCharacters()) {
                    if (id < game->getLobbySize() && c->alive) {
                        if (c->entity->getPosition() != chr->entity->getPosition()) {
                            m_safe = false;
                        } else {
                            game->getLevel().lock()->getMap()->removeEntity(c->entity);
                            c->alive = false;
                            {
                                sf::Packet packet;
                                PlayerDespawnPayload payload;
                                payload.cid = uint8_t(id);
                                packet.append(&payload, sizeof(payload));
                                game->broadcast(packet);
                            }
                        }
                    }
                }
                if (m_safe) {
                    game->sendChatGeneric("Everyone has made it!");
                    game->sendChatGeneric("Entering next Level!");
                    game->signalLevelAdvance();
                    auto ents = level->getMap()->allEntities();
                    for(auto & ent : ents) {
                        if(ent->getType() & LIVING) {
                            level->getMap()->removeEntity(ent);
                            auto living = std::static_pointer_cast<LivingEntity>(ent);
                            if(living->hasHPBar()) {
                                living->getHPBar().lock()->setHP(0);
                            }
                        }
                    }
                }
            }
            m_started = true;
        }
        m_started = chr->entity->hasCastIntent() && !chr->abilities[0]->isCastComplete();
        return !m_started;
    }

    Character Beacon::asCharacter() {
        return Character{0,
                         shared_from_this(),
                         {
                                 std::make_shared<GateUnlock>(shared_from_this())
                         },
                         [=](const std::shared_ptr<OnlineGame> &l, const std::shared_ptr<Character> &c) {
                             return takeTurn(l, c);
                         }
        };
    }

    size_t Beacon::populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame,
                            float tileVerticalOffset) const {
        return LivingEntity::populate(map, array, vertexOffset, 6 - (std::clamp(-getVerticalOffset(), 8.f, 14.f) - 8), tileVerticalOffset);
    }

    size_t Beacon::countSlainEnemies(const std::shared_ptr<OnlineGame> & g) const {
        size_t slain = 0;
        auto game = std::static_pointer_cast<HostGame>(g);
        for (int cid = 0; cid < game->getLobbySize(); cid++) {
            slain += game->getCharacters().at(cid)->entity->enemiesSlain;
        }
        return slain;
    }

    bool GateUnlock::cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) {
        m_complete = false;
        bool wasopen = m_open;
        if (pos == m_user->getPosition()) {
            m_open = true;
            auto ap = std::make_shared<AudioPlayback>(lvl.lock()->getApollo()->lookupAudio("beacon_activate"));
            ap->start(lvl);
        } else {
            m_open = false;
        }
        if(m_open && !wasopen) {
            lvl.lock()->addFrameBeginListener(shared_from_this());
        } else {
            m_complete = true;
        }
        return true;
    }

    GateUnlock::GateUnlock(std::shared_ptr<padi::LivingEntity> user) : Ability(std::move(user)) {

    }

    uint32_t GateUnlock::getAbilityType() const {
        return AbilityType::GateUnlock;
    }

    void GateUnlock::castCancel(const std::weak_ptr<Level> &level) {

    }

    void GateUnlock::castIndicator(const std::weak_ptr<Level> &level) {

    }

    bool GateUnlock::isCastComplete() {
        return m_complete;
    }

    bool GateUnlock::onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) {
        auto level = lvl.lock();
        auto map = level->getMap();
        auto tile = map->getTile(m_user->getPosition());
        if (m_open) {
            m_user->setVerticalOffset(m_user->getVerticalOffset() + std::round((-15 - m_user->getVerticalOffset()) * 0.6f));
        } else {
            m_user->setVerticalOffset(m_user->getVerticalOffset() + std::round((6 - m_user->getVerticalOffset()) * 0.6f) - 8);
        }
        if(frame == 8 && m_open) {
            auto pos = m_user->getPosition();
            auto gateAnim = level->getApollo()->lookupAnim("lightning_hold");
            auto gateFloor = level->getApollo()->lookupAnim("lightning_hold_end");
            auto gate = std::make_shared<padi::EntityColumn>(pos);
            gate->m_animation = gateFloor;
            gate->m_stackAnimation = gateAnim;
            gate->m_stackSize = 8;
            map->addEntity(gate, 69);
            //map->removeEntity(m_user);
            for (auto &d: Neighborhood) {
                map->getTile(pos + d)->setColor(sf::Color::White);
            }
        }
        m_complete = frame == 8;
        return !m_complete;
    }
} // content