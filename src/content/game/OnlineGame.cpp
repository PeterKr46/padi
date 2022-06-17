//
// Created by Peter on 15/06/2022.
//

#include "OnlineGame.h"

#include <utility>
#include <SFML/Network.hpp>

#include "../../level/Level.h"
#include "../../entity/LivingEntity.h"
#include "../../level/LevelGenerator.h"
#include "LocalPlayerTurn.h"
#include "../../level/SpawnEvent.h"
#include "../abilities/Abilities.h"
#include "RemotePlayerTurn.h"
#include "Packets.h"
#include "Character.h"

namespace padi::content {

    OnlineGame::OnlineGame(std::vector<Inbox> sockets, bool hosting, std::string const &name,
                           uint32_t seed)
            : m_lobby({hosting, std::move(sockets)}),
              m_seed(seed),
              m_rand(seed) {
        // Initialization is blocking.
        for (auto &socket: m_lobby.remotes) {
            socket.getSocket().lock()->setBlocking(true);
        }
        propagateLobby(name);
        propagateSeed();
        m_level = LevelGenerator().withSeed(m_seed).withArea({32, 32})
                .withSpritesheet("../media/level_sheet.png")    // TODO
                .withApollo("../media/level.apollo")            // TODO
                .generate();
        m_level->centerView({0, 0});
        m_uiContext.init("../media/ui.apollo", "../media/ui_sheet.png");
        m_crt.setShader(m_uiContext.getApollo()->lookupShader("fpa"));
        initializePlayers();
        // Everything at runtime is non-blocking.
        for (auto &socket: m_lobby.remotes) {
            socket.getSocket().lock()->setBlocking(false);
        }
    }

    std::shared_ptr<padi::Activity> OnlineGame::handoff() {
        return shared_from_this();
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

        if (m_activeChar) {
            if (m_activeChar->controller(shared_from_this(), m_activeChar)) {
                if (m_activeChar->alive) {
                    m_turnQueue.push(m_activeChar);
                }
                m_activeChar = m_turnQueue.front();
                m_turnQueue.pop();
                if (m_activeChar->entity) {
                    m_level->centerView(m_activeChar->entity->getPosition());
                    m_level->moveCursor(m_activeChar->entity->getPosition());
                }
            }
        }

        m_crt.asTarget()->draw(m_uiContext);
        target->draw(m_crt);
    }

    void OnlineGame::initializePlayers() {
        if (m_lobby.isHost) {
            initializePlayersHost();
        } else {
            initializePlayersClient();
        }
        m_activeChar = m_turnQueue.front();
        m_turnQueue.pop();
    }

    void OnlineGame::assignPlayerAbility(PlayerAssignAbilityPayload &payload) {
        auto &chr = m_characters.at(payload.playerId);
        auto &abilities = chr->abilities;
        // TODO robust pls
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
            default: {
                printf("[OnlineGame] Attempted to assign unknown ability type.\n");
            }
        }
    }

    void OnlineGame::propagateSeed() {
        if (m_lobby.isHost) {
            propagateSeedHost();
        } else {
            propagateSeedClient();
        }
    }


    void OnlineGame::propagateLobby(std::string const &name) {
        if (m_lobby.isHost) {
            propagateLobbyHost(name);
        } else {
            propagateLobbyClient(name);
        }
    }

    void OnlineGame::close() {
        for (auto &socket: m_lobby.remotes) {
            socket.getSocket().lock()->disconnect();
        }
        while (!m_turnQueue.empty()) m_turnQueue.pop();
        m_activeChar.reset();
    }

    std::shared_ptr<Level> OnlineGame::getLevel() const {
        return m_level;
    }

} // content