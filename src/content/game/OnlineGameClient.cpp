//
// Created by Peter on 17/06/2022.
//

#include "OnlineGame.h"
#include "LocalPlayerTurn.h"
#include "Character.h"
#include "RemotePlayerTurn.h"
#include "../../entity/LivingEntity.h"
#include "../../level/SpawnEvent.h"

namespace padi::content {


    void OnlineGame::propagateSeedClient() {
        sf::Packet packet;
        GameSeedPayload gameSeedPL;
        printf("[OnlineGame|Client] Receiving seed!\n");
        auto host = m_lobby.sockets.front();
        auto status = host->receive(packet);
        if (status != sf::Socket::Done) {
            printf("[OnlineGame|Client] Error occurred while receiving seed!\n");
            exit(-1);
        }
        ReconstructPayload(packet, gameSeedPL);
        m_seed = gameSeedPL.seed;
        m_rand = std::mt19937(m_seed);
        printf("[OnlineGame|Client] Received seed %u!\n", m_seed);
    }

    void OnlineGame::initializePlayersClient() {
        auto apollo = m_level->getApollo();
        sf::Packet packet;
        PlayerSpawnPayload playerSpawnPL;
        PlayerAssignAbilityPayload playerAbilityPL;
        std::shared_ptr<Character> player;
        LocalPlayerTurn localPlayerTurn(&m_uiContext, m_lobby.sockets);
        auto &host = m_lobby.sockets.front();
        RemotePlayerTurn remotePlayerTurn(host);
        for (size_t id = 0; id < m_lobby.sockets.size() + 1; ++id) {
            auto status = host->receive(packet);
            if (status != sf::Socket::Done) {
                printf("[OnlineGame|Client] Error occurred while receiving spawn event!\n");
                exit(-1);
            }
            ReconstructPayload(packet, playerSpawnPL);
            player = std::make_shared<Character>(Character{uint32_t(playerSpawnPL.id)});
            m_characters[id] = player;

            player->entity = std::make_shared<padi::LivingEntity>(
                    m_lobby.names[id],
                    apollo->lookupAnimContext("cube"),
                    playerSpawnPL.pos
            );
            player->entity->setColor(playerSpawnPL.color);

            for(int i = 0; i < 4; ++i) {
                status = host->receive(packet);
                if (status != sf::Socket::Done) {
                    printf("[OnlineGame|Client] Error occurred while receiving spawn event!\n");
                    exit(-1);
                }
                ReconstructPayload(packet, playerAbilityPL);
                assignPlayerAbility(playerAbilityPL);
            }

            if (playerSpawnPL.local) {
                player->controller = [=](const std::shared_ptr<OnlineGame> &l,
                                         const std::shared_ptr<Character> &c) mutable {
                    return localPlayerTurn(l, c);
                };
            } else {
                player->controller = [=](const std::shared_ptr<OnlineGame> &l,
                                         const std::shared_ptr<Character> &c) mutable {
                    return remotePlayerTurn(l, c);
                };
            }

            auto spawnEvent = std::make_shared<padi::SpawnEvent>(player->entity);
            spawnEvent->dispatch(m_level);
            m_turnQueue.push(player);
        }

    }

    void OnlineGame::propagateLobbyClient(const std::string &basicString) {
        sf::Packet packet;
        LobbySizePayload lobbySizePL;
        PlayerNamePayload namePL;
        // CLIENT   receive lobby size
        // CLIENT   send your name
        // CLIENT   receive all names
        printf("[OnlineGame|Client] Receiving lobby size!\n");
        auto host = m_lobby.sockets.front();
        auto status = host->receive(packet);
        if (status != sf::Socket::Done) {
            printf("[OnlineGame|Client] Error occurred while receiving seed!\n");
            exit(-1);
        }
        ReconstructPayload(packet, lobbySizePL);
        printf("[OnlineGame|Client] Received lobby size: %hhu!\n", lobbySizePL.players);
        printf("[OnlineGame|Client] Sending own name!\n");
        m_lobby.names.resize(lobbySizePL.players, "");
        std::memcpy(&namePL.name, basicString.c_str(), std::min(8ull, basicString.length()));
        PackagePayload(packet, namePL);
        host->send(packet);
        printf("[OnlineGame|Client] Sent own name!\n");
        for (size_t id = 0; id < m_lobby.names.size() - 1; ++id) {
            status = host->receive(packet);
            if (status != sf::Socket::Done) {
                printf("[OnlineGame|Client] Error occurred while receiving name!\n");
                exit(-1);
            }
            ReconstructPayload(packet, namePL);
            printf("[OnlineGame|Client] Received name %hhu: %.*s!\n", namePL.player, 8, namePL.name);
            m_lobby.names[namePL.player] = std::string(namePL.name, std::min(strlen(namePL.name), 8ull));
        }
        printf("[OnlineGame|Client] Received all names!\n");
    }
}