//
// Created by Peter on 15/06/2022.
//

#include "OnlineGame.h"

#include <utility>

#include "../../level/Level.h"
#include "../../entity/LivingEntity.h"
#include "../../level/LevelGenerator.h"
#include "../../level/SpawnEvent.h"
#include "../../Controls.h"
#include "../abilities/Abilities.h"
#include "LocalPlayerTurn.h"
#include "RemotePlayerTurn.h"
#include "Character.h"
#include "SFML/Window/Keyboard.hpp"


namespace padi::content {

    OnlineGame::OnlineGame(std::vector<Inbox> sockets, bool hosting, std::string const &name,
                           uint32_t seed)
            : m_lobby({hosting, std::move(sockets)}),
              m_seed(seed),
              m_rand(seed),
              m_chat({250,180, 200, 60}) {
        propagateLobby(name);
        propagateSeed();
        m_level = LevelGenerator().withSeed(m_seed).withArea({32, 32})
                .withSpritesheet("../media/level_sheet.png")    // TODO
                .withApollo("../media/level.apollo")            // TODO
                .generate();
        m_level->centerView({0, 0});
        m_uiContext.init("../media/ui.apollo", "../media/ui_sheet.png");

        m_chat.init(&m_uiContext);
        m_chat.submit = [&](std::string const& msg) {
            ChatMessagePayload msgPayload;
            std::memcpy(msgPayload.message, msg.c_str(), std::min(msg.size(), 32ull));
            sf::Packet packet = PackagePayload(msgPayload);

            for(auto & client : m_lobby.remotes) {
                client.getSocket().lock()->send(packet);
            }
            if(m_lobby.isHost) {
                m_chat.write(&m_uiContext, msg);
            }
            m_uiContext.setFocusActive(false);
        };

        m_crt.setShader(m_uiContext.getApollo()->lookupShader("fpa"));
        initializePlayers();
    }

    std::weak_ptr<padi::Activity> OnlineGame::handoff() {
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
        m_chat.draw(&m_uiContext);
        if(!m_uiContext.isFocusActive() && padi::Controls::wasKeyReleased(sf::Keyboard::T)) {
            m_uiContext.setFocusActive(true);
        } else if(m_uiContext.isFocusActive() && padi::Controls::wasKeyReleased(sf::Keyboard::Escape)) {
            m_uiContext.setFocusActive(false);
        }

        m_crt.asTarget()->draw(m_uiContext);
        target->draw(m_crt);

        if (m_lobby.isHost) updateHost();
        else updateClient();
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

    std::weak_ptr<Level> OnlineGame::getLevel() const {
        return m_level;
    }

    void OnlineGame::updateClient() {
        auto &host = m_lobby.remotes.front();
        if (host.fetch() == -1) {
            exit(-1);
        }
        if (host.has(PayloadType::CharacterSpawn)) {
            PlayerSpawnPayload payload;
            host.check(payload);
            if (m_characters.find(payload.id) != m_characters.end()) {
                // TODO
            } else {

            }
        }
        if(host.has(PayloadType::ChatMessage)) {
            ChatMessagePayload payload;
            host.check(payload);
            m_chat.write(&m_uiContext, std::string(payload.message, std::min(32ull, strlen(payload.message))));
        }
    }

    void OnlineGame::updateHost() {
        for (size_t cid = 0; cid < m_lobby.remotes.size(); ++cid) {
            auto &client = m_lobby.remotes[cid];
            if(client) {
                if (client.fetch() == -1) {
                    printf("[OnlineGame|Server] Client %zull (%s) lost connection.\n", cid, m_lobby.names[cid].c_str());
                    m_lobby.remotes[cid] = Inbox(); // TODO how do i handle this?
                } else {
                    if(client.has(PayloadType::ChatMessage)) {
                        ChatMessagePayload payload;
                        client.check(payload);
                        sf::Packet packet = PackagePayload(payload);
                        for(auto & sock : m_lobby.remotes) {
                            sock.getSocket().lock()->send(packet);
                        }
                        m_chat.write(&m_uiContext, std::string(payload.message, std::min(32ull, strlen(payload.message))));
                    }
                }
            }
        }

    }

} // content