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

    OnlineGame::OnlineGame(std::vector<InOutBox> sockets, bool hosting, std::string const &name,
                           uint32_t seed)
            : m_lobby({hosting, std::move(sockets)}),
              m_seed(seed),
              m_rand(seed),
              m_chat({250, 194, 200, 60}) {
        propagateLobby(name);
        propagateSeed();
        m_level = LevelGenerator().withSeed(m_seed).withArea({32, 32})
                .withSpritesheet("../media/level_sheet.png")    // TODO
                .withApollo("../media/level.apollo")            // TODO
                .generate();
        m_level->centerView({0, 0});
        m_uiContext.init("../media/ui.apollo", "../media/ui_sheet.png");
        m_uiContext.setFocusActive(false);
        m_chat.init(&m_uiContext);
        m_chat.write(&m_uiContext, "Press T to chat.");
        m_chat.submit = [&](std::string const &msg) {
            ChatMessagePayload msgPayload;
            std::memcpy(msgPayload.message, msg.c_str(), std::min(msg.size(), 32ull));
            sf::Packet packet = PackagePayload(msgPayload);

            for(auto & client : m_lobby.remotes) {
                client.send(packet);
            }
            if (m_lobby.isHost) {
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

        takeTurn();

        m_chat.draw(&m_uiContext);
        if (!m_uiContext.isFocusActive() && padi::Controls::wasKeyReleased(sf::Keyboard::T)) {
            m_uiContext.setFocusActive(true);
        } else if (m_uiContext.isFocusActive() && padi::Controls::wasKeyReleased(sf::Keyboard::Escape)) {
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
            case AbilityType::Peep: {
                abilities[payload.abilitySlot] = std::make_shared<padi::content::Peep>(chr->entity);
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
        for (auto &remote: m_lobby.remotes) {
            auto socket = remote.getSocket().lock();
            if(socket) socket->disconnect();
        }
        while (!m_turnQueue.empty()) m_turnQueue.pop();
        m_activeChar.reset();
    }

    std::weak_ptr<Level> OnlineGame::getLevel() const {
        return m_level;
    }

    void OnlineGame::updateClient() {
        auto &host = m_lobby.remotes.front();
        if (host.receive() == -1) {
            exit(-1);
        }
        if (host.has(PayloadType::CharacterSpawn)) {
            PlayerSpawnPayload payload;
            host.fetch(payload);
            if (m_characters.find(payload.id) != m_characters.end()) {
                // TODO
            } else {

            }
        }
        if (host.has(PayloadType::ChatMessage)) {
            ChatMessagePayload payload;
            host.fetch(payload);
            m_chat.write(&m_uiContext, std::string(payload.message, std::min(32ull, strlen(payload.message))));
        }
    }

    void OnlineGame::updateHost() {
        for (size_t cid = 0; cid < m_lobby.remotes.size(); ++cid) {
            auto &client = m_lobby.remotes[cid];
            if (client) {
                if (client.receive() == -1) {
                    printf("[OnlineGame|Server] Client %zull (%s) lost connection.\n", cid, m_lobby.names[cid].c_str());
                    m_lobby.remotes[cid] = InOutBox(); // TODO how do i handle this?
                    m_level->getMap()->removeEntity(m_characters[cid]->entity);
                    m_chat.submit(m_characters[cid]->entity->getName() + " lost connection.");
                } else {
                    if (client.has(PayloadType::ChatMessage)) {
                        ChatMessagePayload payload;
                        client.fetch(payload);
                        sf::Packet packet = PackagePayload(payload);
                        for (auto &remote: m_lobby.remotes) {
                            remote.send(packet);
                        }
                        m_chat.write(&m_uiContext,
                                     std::string(payload.message, std::min(32ull, strlen(payload.message))));
                    }
                }
            }
        }

    }

    void OnlineGame::takeTurn() {
        if (m_activeChar) {
            if (m_activeChar->controller(shared_from_this(), m_activeChar)) {
                // Turn complete.
                if(m_lobby.isHost) advanceTurnHost();
                else m_activeChar.reset();
            }
        } else if (m_lobby.isHost) {
            advanceTurnHost();
        } else {
            auto &host = m_lobby.remotes.front();
            CharacterTurnBeginPayload nextTurn(0);
            if (host.fetch(nextTurn)) {
                printf("[OnlineGame|Client] Character %u is starting their turn.\n", nextTurn.characterId);
                m_activeChar = m_characters.at(nextTurn.characterId);
                if (m_activeChar->entity) {
                    m_level->centerView(m_activeChar->entity->getPosition());
                    m_level->moveCursor(m_activeChar->entity->getPosition());
                }
            }
        }
    }

    void OnlineGame::advanceTurnHost() {
        if (m_lobby.isHost && !m_turnQueue.empty()) {
            if (m_activeChar && m_activeChar->alive) {
                m_turnQueue.push(m_activeChar->id);
            }
            printf("[OnlineGame|Server] Character %u is starting their turn.\n", m_turnQueue.front());
            m_activeChar = m_characters.at(m_turnQueue.front());
            m_turnQueue.pop();
            if (m_activeChar->entity) {
                m_level->centerView(m_activeChar->entity->getPosition());
                m_level->moveCursor(m_activeChar->entity->getPosition());
            }
            if(m_activeChar->id < m_lobby.remotes.size()) {
                auto packet = PackagePayload(ChatMessagePayload{ChatMessage, "Your turn.\0"});
                m_lobby.remotes[m_activeChar->id].send(packet);
            } else if(m_activeChar->id == m_lobby.remotes.size() || (m_activeChar->id == 0 && m_lobby.remotes.empty())) {
                m_chat.write(&m_uiContext, "Your turn.");
            }
            auto packet = PackagePayload(CharacterTurnBeginPayload(m_activeChar->id));
            for (auto &remote: m_lobby.remotes) {
                remote.send(packet);
            }
        }
    }

} // content