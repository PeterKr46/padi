//
// Created by Peter on 15/06/2022.
//

#include "OnlineGame.h"


#include "../../entity/LivingEntity.h"
#include "../../level/LevelGenerator.h"
#include "../../Controls.h"
#include "../abilities/Abilities.h"
#include "Character.h"
#include "SFML/Window/Keyboard.hpp"
#include "../../media/AudioPlayback.h"
#include "../npc/ExplosiveMob.h"
#include "../npc/SlugMob.h"
#include "../npc/Beacon.h"


namespace padi::content {

    std::weak_ptr<padi::Activity> OnlineGame::handoff() {
        if (!m_next) m_next = shared_from_this();
        return m_next;
    }

    void OnlineGame::handleResize(int width, int height) {
        m_crt.handleResize(width, height);
    }

    void OnlineGame::draw(sf::RenderTarget *target) {
        m_level->update(m_crt.asTarget());

        m_level->populateVBO();
        m_crt.asTarget()->clear();
        auto states = sf::RenderStates::Default;

        if(padi::Controls::isKeyDown(sf::Keyboard::Tab)) {
            states.transform.scale(2/12.f, 2/12.f);
            m_crt.asTarget()->setView(sf::View({0,0},{453,255}));
        }

        states.transform.scale(255.f / m_crt.asTarget()->getView().getSize().y,
                               255.f / m_crt.asTarget()->getView().getSize().y
                               );
        m_crt.asTarget()->draw(*m_level, states);
        m_uiContext.nextFrame();

        update();

        m_chat.ui.draw(&m_uiContext);
        if (!m_uiContext.isFocusActive() && padi::Controls::wasKeyReleased(sf::Keyboard::T)) {
            m_uiContext.setFocusActive(true);
        } else if (m_uiContext.isFocusActive() && padi::Controls::wasKeyReleased(sf::Keyboard::Escape)) {
            m_uiContext.setFocusActive(false);
        }

        m_crt.paused = m_level->isPaused();
        m_crt.asTarget()->draw(m_uiContext);
        target->draw(m_crt);
    }

    void OnlineGame::assignPlayerAbility(CharacterAbilityAssignPayload &payload) {
        auto &chr = m_characters.at(payload.cid);
        auto &abilities = chr->abilities;
        if (abilities.size() <= payload.abilitySlot) {
            abilities.resize(payload.abilitySlot + 1, {nullptr});
        }
        printf("[OnlineGame] Assigning Ability (%i, %i) to character %i\n", payload.abilityType, *payload.abilityProps,
               payload.cid);
        switch (payload.abilityType) {
            case AbilityType::Walk: {
                uint8_t range = payload.abilityProps[0];
                int16_t walkable = *reinterpret_cast<int16_t *>(payload.abilityProps + 1);
                abilities[payload.abilitySlot] = std::make_shared<padi::content::Walk>(
                        chr->entity,
                        range,
                        Walk::Walkable{walkable}
                );
                break;
            }
            case AbilityType::Teleport: {
                abilities[payload.abilitySlot] = std::make_shared<padi::content::Teleport>(chr->entity);
                break;
            }
            case AbilityType::Lighten: {
                abilities[payload.abilitySlot] = std::make_shared<padi::content::Lighten>(chr->entity);
                break;
            }
            case AbilityType::Darken: {
                abilities[payload.abilitySlot] = std::make_shared<padi::content::Darken>(chr->entity);
                break;
            }
            case AbilityType::Dash: {
                uint8_t range = payload.abilityProps[0];
                int16_t walkable = *reinterpret_cast<int16_t *>(payload.abilityProps + 1);
                abilities[payload.abilitySlot] = std::make_shared<padi::content::Dash>(
                        chr->entity,
                        range,
                        Walk::Walkable{walkable}
                );
                break;
            }
            case AbilityType::Peep: {
                abilities[payload.abilitySlot] = std::make_shared<padi::content::Peep>(chr->entity);
                break;
            }
            case AbilityType::SelfDestruct: {
                abilities[payload.abilitySlot] = std::make_shared<padi::content::SelfDestruct>(chr->entity);
                break;
            }
            case AbilityType::SlugWalk: {
                uint8_t range = payload.abilityProps[0];
                int16_t walkable = *reinterpret_cast<int16_t *>(payload.abilityProps + 1);
                abilities[payload.abilitySlot] = std::make_shared<padi::content::SlugWalk>(
                        chr->entity,
                        range,
                        Walk::Walkable{walkable}
                );
                break;
            }
            case AbilityType::GateUnlock: {
                abilities[payload.abilitySlot] = std::make_shared<GateUnlock>(chr->entity);
                break;
            }
            case AbilityType::Raze: {
                abilities[payload.abilitySlot] = std::make_shared<Raze>(chr->entity);
                break;
            }
            default: {
                printf("[OnlineGame] Attempted to assign unknown ability type.\n");
            }
        }
    }

    std::weak_ptr<Level> OnlineGame::getLevel() const {
        return m_level;
    }

    void OnlineGame::printChatMessage(const std::string &msg, bool notify) {
        if (notify) {
            if (!m_chat.notification) {
                m_chat.notification = std::make_shared<padi::AudioPlayback>(
                        m_uiContext.getApollo()->lookupAudio("chat_msg"));
                m_chat.notification->start(m_level);
            }
            m_chat.notification->restart(m_level);
        }
        m_chat.ui.write(&m_uiContext, msg);
    }

    void OnlineGame::synchronize(std::string const &ownName) {
        m_stage++;
        synchronizeLobby(ownName);
        synchronizeSeed();
        // This glorious copy prevents Apollo from going out of scope
        auto tmpCopy = m_level;

        m_level = LevelGenerator().withSeed(m_seed).withArea({int(std::log10(m_stage * 10)) * 24, int(std::log10(m_stage * 10)) * 24})
                .withSpritesheet("../media/level_sheet.png")    // TODO
                .withApollo("../media/level.apollo")            // TODO
                .generateLevel();
        m_level->centerView({0, 0});
        m_uiContext.init("../media/ui.apollo", "../media/ui_sheet.png");
        m_uiContext.setFocusActive(false);
        m_chat.ui.init(&m_uiContext);
        m_chat.ui.write(&m_uiContext, "Press T to chat.");
        m_chat.ui.submit = [&](std::string const &msg) {
            sendChatMessage(msg);
            m_uiContext.setFocusActive(false);
        };
        m_crt.setShader(m_uiContext.getApollo()->lookupShader("fpa"));
        initializeCharacters();
    }

    size_t OnlineGame::getSeed() const {
        return m_seed;
    }

    const std::map<uint32_t, std::shared_ptr<Character>, std::less<>> &OnlineGame::getCharacters() const {
        return m_characters;
    }

    padi::UIContext* OnlineGame::getUIContext() {
        return &m_uiContext;
    }

    void OnlineGame::spawnEvent(sf::Vector2i const& pos, EventSpawnPayload const& payload) {
        auto entity = std::make_shared<StaticEntity>(payload.pos, EVENT);
        entity->m_animation = m_level->getApollo()->lookupAnim("q_mark");
        m_level->getMap()->addEntity(entity);
        auto & data = m_events[pos];
        data.first = entity;
        data.second.pos = pos;
        data.second.abilityType = payload.abilityType;
        std::memcpy(data.second.abilityProps, payload.abilityProps, sizeof(payload.abilityProps));
    }

    void OnlineGame::despawnEvent(sf::Vector2i const & pos) {
        auto map = m_level->getMap();
        auto entity = m_events.at(pos);
        map->removeEntity(entity.first);
        m_events.erase(pos);
    }


} // content