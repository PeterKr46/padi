//
// Created by Peter on 17/06/2022.
//

#include "OnlineGame.h"
#include "LocalPlayerTurn.h"
#include "../../entity/LivingEntity.h"
#include "../abilities/Abilities.h"
#include "RemotePlayerTurn.h"
#include "../../level/SpawnEvent.h"
#include "Character.h"

namespace padi::content {
    void OnlineGame::propagateSeedHost() {
        sf::Packet packet;
        GameSeedPayload gameSeedPL;
        gameSeedPL.seed = m_seed;
        PackagePayload(packet, gameSeedPL);
        printf("[OnlineGame|Server] Propagating seed!\n");
        for (auto &socket: m_lobby.sockets) {
            socket->send(packet);
        }
        printf("[OnlineGame|Server] Propagated seed %u!\n", m_seed);
    }

    void OnlineGame::initializePlayersHost() {
        auto apollo = m_level->getApollo();
        sf::Packet packet;
        PlayerSpawnPayload playerSpawnPL;
        PlayerAssignAbilityPayload playerAbilityPL;
        std::shared_ptr<Character> player;
        LocalPlayerTurn localPlayerTurn(&m_uiContext, m_lobby.sockets);
        for (size_t id = 0; id < m_lobby.sockets.size() + 1; ++id) {
            playerSpawnPL.id = id;
            playerSpawnPL.pos = sf::Vector2i{int(id), 0};
            playerSpawnPL.color = sf::Color(m_rand()); // TODO rand progression!
            playerSpawnPL.color.a = 255;
            for (size_t sockId = 0; sockId < m_lobby.sockets.size(); ++sockId) {
                playerSpawnPL.local = sockId == id;
                m_lobby.sockets[sockId]->send(PackagePayload(packet, playerSpawnPL));
            }
            player = std::make_shared<Character>(Character{uint32_t(id)});
            m_characters[id] = player;

            player->entity = std::make_shared<padi::LivingEntity>(
                    m_lobby.names[id],
                    apollo->lookupAnimContext("cube"),
                    playerSpawnPL.pos
            );
            player->entity->setColor(playerSpawnPL.color);

            // Just Walk for now...
            playerAbilityPL.abilityType = AbilityType::Walk;
            playerAbilityPL.abilityProps[0] = 3;
            playerAbilityPL.abilitySlot = 0;
            playerAbilityPL.playerId = id;
            for (auto &socket: m_lobby.sockets) {
                socket->send(PackagePayload(packet, playerAbilityPL));
            }
            assignPlayerAbility(playerAbilityPL);

            if (id < m_lobby.sockets.size()) {
                RemotePlayerTurn remotePlayerTurn(m_lobby.sockets[id]);
                player->controller = [=](const std::shared_ptr<OnlineGame> &l,
                                         const std::shared_ptr<Character> &c) mutable {
                    return remotePlayerTurn(l, c);
                };
            }

            auto spawnEvent = std::make_shared<padi::SpawnEvent>(player->entity);
            spawnEvent->dispatch(m_level);
            m_turnQueue.push(player);
        }
        m_turnQueue.back()->controller = [=](const std::shared_ptr<OnlineGame> &l,
                                             const std::shared_ptr<Character> &c) mutable {
            return localPlayerTurn(l, c);
        };
    }

    void OnlineGame::propagateLobbyHost(const std::string &basicString) {
        sf::Packet packet;
        LobbySizePayload lobbySizePL;
        PlayerNamePayload namePL;
        // HOST     propagate lobby size
        // HOST     receive all names
        // HOST     propagate all names
        printf("[OnlineGame|Server] Propagating lobby size!\n");
        lobbySizePL.players = m_lobby.sockets.size() + 1;
        m_lobby.names.resize(lobbySizePL.players, "");
        PackagePayload(packet, lobbySizePL);
        for (auto &socket: m_lobby.sockets) {
            socket->send(packet);
        }

        printf("[OnlineGame|Server] Receiving lobby names!\n");
        for (size_t id = 0; id < m_lobby.sockets.size(); ++id) {
            auto &socket = m_lobby.sockets[id];
            auto status = socket->receive(packet);
            if (status != sf::Socket::Done) {
                printf("[OnlineGame|Server] Error occurred while receiving name!\n");
                exit(-1);
            }
            ReconstructPayload(packet, namePL);
            namePL.player = id;
            m_lobby.names[id] = std::string(namePL.name, std::min(strlen(namePL.name), 8ull));
            printf("[OnlineGame|Server] Received name %zu: %.*s!\n", id, 8, namePL.name);
        }
        for (size_t id = 0; id < m_lobby.sockets.size(); ++id) {

            auto &playerName = m_lobby.names[id];
            namePL.player = id;
            std::memcpy(&namePL.name, playerName.c_str(), std::min(8ull, playerName.length()));
            PackagePayload(packet, namePL);

            for (size_t sockId = 0; sockId < m_lobby.sockets.size(); ++sockId) {
                if (sockId != id) {
                    m_lobby.sockets[sockId]->send(packet);
                }
            }
        }
        std::memcpy(&namePL.name, basicString.c_str(), std::min(8ull, basicString.length()));
        namePL.player = m_lobby.sockets.size();
        printf("[OnlineGame|Server] Propagating own name %i: %.*s!\n", namePL.player, 8, namePL.name);
        m_lobby.names.back() = basicString;
        PackagePayload(packet, namePL);
        for (auto &socket: m_lobby.sockets) {
            socket->send(packet);
        }
    }
}