//
// Created by Peter on 15/06/2022.
//

#include "OnlineGame.h"

#include <utility>

#include "../../entity/LivingEntity.h"
#include "../../level/LevelGenerator.h"
#include "../../Controls.h"
#include "../abilities/Abilities.h"
#include "LocalPlayerTurn.h"
#include "RemotePlayerTurn.h"
#include "Character.h"
#include "SFML/Window/Keyboard.hpp"
#include "../menu/MainMenu.h"
#include "../vfx/MapShaker.h"
#include "../../media/AudioPlayback.h"
#include "../npc/Mob.h"


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
        states.transform.scale(255.f / m_crt.asTarget()->getView().getSize().y,
                               255.f / m_crt.asTarget()->getView().getSize().y);
        m_crt.asTarget()->draw(*m_level, states);
        m_uiContext.nextFrame();

        update();

        m_chat.ui.draw(&m_uiContext);
        if (!m_uiContext.isFocusActive() && padi::Controls::wasKeyReleased(sf::Keyboard::T)) {
            m_uiContext.setFocusActive(true);
        } else if (m_uiContext.isFocusActive() && padi::Controls::wasKeyReleased(sf::Keyboard::Escape)) {
            m_uiContext.setFocusActive(false);
        }

        m_crt.asTarget()->draw(m_uiContext);
        target->draw(m_crt);
    }

    void OnlineGame::assignPlayerAbility(PlayerAssignAbilityPayload &payload) {
        auto &chr = m_characters.at(payload.character);
        auto &abilities = chr->abilities;
        if (abilities.size() <= payload.abilitySlot) {
            abilities.resize(payload.abilitySlot + 1, {nullptr});
        }

        switch (payload.abilityType) {
            case AbilityType::Walk: {
                abilities[payload.abilitySlot] = std::make_shared<padi::content::Walk>(
                        chr->entity,
                        payload.abilityProps[0]);
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
                abilities[payload.abilitySlot] = std::make_shared<padi::content::Dash>(
                        chr->entity,
                        payload.abilityProps[0]);
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
            default: {
                printf("[OnlineGame] Attempted to assign unknown ability type.\n");
            }
        }
    }

    std::weak_ptr<Level> OnlineGame::getLevel() const {
        return m_level;
    }
    void OnlineGame::printChatMessage(const std::string &msg) {
        if(!m_chat.notification) {
            m_chat.notification = std::make_shared<padi::AudioPlayback>(m_uiContext.getApollo()->lookupAudio("chat_msg"));
        }
        m_chat.notification->restart(m_level.get());
        m_chat.ui.write(&m_uiContext, msg);
    }

    void OnlineGame::synchronize(std::string const &ownName) {
        synchronizeLobby(ownName);
        synchronizeSeed();
        m_level = LevelGenerator().withSeed(m_seed).withArea({32, 32})
                .withSpritesheet("../media/level_sheet.png")    // TODO
                .withApollo("../media/level.apollo")            // TODO
                .generate();
        m_level->centerView({0, 0});
        m_uiContext.init("../media/ui.apollo", "../media/ui_sheet.png");
        m_uiContext.setFocusActive(false);
        m_chat.ui.init(&m_uiContext);
        m_chat.ui.write(&m_uiContext, "Press T to window.");
        m_chat.ui.submit = [&](std::string const &msg) {
            sendChatMessage(msg);
        };
        m_crt.setShader(m_uiContext.getApollo()->lookupShader("fpa"));
        initializeCharacters();
        m_level->addFrameBeginListener(std::make_shared<MapShaker>());
    }

} // content