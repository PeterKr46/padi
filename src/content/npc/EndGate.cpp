//
// Created by Peter on 22/06/2022.
//

#include "EndGate.h"
#include "../game/OnlineGame.h"

#include <utility>

namespace padi::content {
        EndGate::EndGate(std::string name, const padi::AnimationSet *moveset, const sf::Vector2i &pos) : LivingEntity(
                std::move(name), moveset, pos) {

        }

        bool EndGate::takeTurn(const std::shared_ptr<OnlineGame> & g, const std::shared_ptr<Character> & chr) {

            auto game = std::static_pointer_cast<HostGame>(g);
            auto level = game->getLevel().lock();

            sf::Vector2i target = chr->entity->getPosition();
            if(!m_open) {
                size_t killed = 0;
                for (auto &[id, c]: game->getCharacters()) {
                    if (id >= game->getLobbySize() && !c->alive) ++killed;
                }
                if (killed < m_requiredKills) {
                    game->sendChatGeneric("Clear " + std::to_string(m_requiredKills - killed) + " enemies!");
                    target.x++;
                } else {
                    m_open = true;
                    game->sendChatGeneric("THE GATE IS OPEN!");
                    game->sendChatGeneric("Follow the light.");
                }
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
            if(m_open) {
                bool safe = true;
                for (auto &[id, c]: game->getCharacters()) {
                    if (id < game->getLobbySize() && c->alive) {
                        if (c->entity->getPosition() != chr->entity->getPosition()) {
                            safe = false;
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
                if (safe) {
                    game->sendChatGeneric("Everyone has made it.");
                    game->sendChatGeneric("(Level Over)");
                }
            }
            return true;
        }

        Character EndGate::asCharacter(uint32_t id) {
            return Character{id,
                             shared_from_this(),
                             {
                                     std::make_shared<GateUnlock>(shared_from_this())
                             },
                             [=](const std::shared_ptr<OnlineGame> &l, const std::shared_ptr<Character> &c) {
                                 return takeTurn(l, c);
                             }
            };
        }

        bool GateUnlock::cast(const std::weak_ptr<Level> &lvl, const sf::Vector2i &pos) {
            if(pos == m_user->getPosition()) {
                auto map = lvl.lock()->getMap();
                auto gateAnim = lvl.lock()->getApollo()->lookupAnim("lightning_hold");
                auto gateFloor = lvl.lock()->getApollo()->lookupAnim("lightning_hold_end");
                auto gate = std::make_shared<padi::EntityColumn>(pos);
                gate->m_animation = gateFloor;
                gate->m_stackAnimation = gateAnim;
                gate->m_stackSize = 8;
                //gate->setVerticalOffset(-8);
                map->addEntity(gate);
                map->removeEntity(m_user);
                for(auto & d : Neighborhood) {
                    map->getTile(pos + d)->setColor(sf::Color::White);
                }
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
            return true;
        }
    } // content