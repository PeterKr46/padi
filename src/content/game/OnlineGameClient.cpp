//
// Created by Peter on 17/06/2022.
//

#include "OnlineGame.h"
#include "LocalPlayerTurn.h"
#include "Character.h"
#include "RemotePlayerTurn.h"
#include "../../entity/LivingEntity.h"
#include "../../level/SpawnEvent.h"
#include "../menu/MainMenu.h"

namespace padi::content {


    void ClientGame::synchronizeSeed() {
        GameSeedPayload gameSeedPL;
        printf("[OnlineGame|Client] Receiving seed!\n");
        while (!m_lobby.host.fetch(gameSeedPL)) {
            if (m_lobby.host.receive() == -1) {
                printf("[OnlineGame|Client] Lost connection!\n");
                exit(-1);
            }
        }
        m_seed = gameSeedPL.seed;
        m_rand = std::mt19937(m_seed);
        printf("[OnlineGame|Client] Received seed %u!\n", m_seed);
    }

    void ClientGame::initializeCharacters() {
        auto apollo = m_level->getApollo();
        PlayerSpawnPayload playerSpawnPL;
        PlayerAssignAbilityPayload playerAbilityPL;
        std::shared_ptr<Character> player;
        std::vector<InOutBox> remotes = {m_lobby.host};
        LocalPlayerTurn localPlayerTurn(&m_uiContext, remotes);
        RemotePlayerTurn remotePlayerTurn(m_lobby.host);
        for (size_t id = 0; id < m_lobby.names.size(); ++id) {
            while (!m_lobby.host.fetch(playerSpawnPL)) {
                if (m_lobby.host.receive() == -1) {
                    printf("[OnlineGame|Client] Lost connection!\n");
                    exit(-1);
                }
            }
            player = std::make_shared<Character>(Character{uint32_t(playerSpawnPL.id)});
            m_characters[id] = player;

            player->entity = std::make_shared<padi::LivingEntity>(
                    m_lobby.names[id],
                    apollo->lookupAnimContext("cube"),
                    playerSpawnPL.pos
            );
            player->entity->setColor(playerSpawnPL.color);

            for (int i = 0; i < 4; ++i) {
                while (!m_lobby.host.fetch(playerAbilityPL)) {
                    if (m_lobby.host.receive() == -1) {
                        printf("[OnlineGame|Client] Lost connection!\n");
                        exit(-1);
                    }
                }
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
        }

    }

    void ClientGame::synchronizeLobby(const std::string &ownName) {
        sf::Packet packet;
        LobbySizePayload lobbySizePL;
        PlayerNamePayload namePL;
        // CLIENT   receive lobby size
        // CLIENT   send your name
        // CLIENT   receive all names
        printf("[OnlineGame|Client] Receiving lobby size!\n");
        while (!m_lobby.host.fetch(lobbySizePL)) {
            if (m_lobby.host.receive() == -1) {
                printf("[OnlineGame|Client] Lost connection!\n");
                exit(-1);
            }
        }
        printf("[OnlineGame|Client] Received lobby size: %hhu!\n", lobbySizePL.players);
        m_lobby.size = lobbySizePL.players;
        m_lobby.names.resize(m_lobby.size, "");
        printf("[OnlineGame|Client] Sending own name!\n");
        std::memcpy(&namePL.name, ownName.c_str(), std::min(8ull, ownName.length()));
        PackagePayload(packet, namePL);
        m_lobby.host.send(packet);
        printf("[OnlineGame|Client] Sent own name!\n");
        for (size_t id = 0; id < m_lobby.names.size() - 1; ++id) {
            while (!m_lobby.host.fetch(namePL)) {
                if (m_lobby.host.receive() == -1) {
                    printf("[OnlineGame|Client] Lost connection!\n");
                    exit(-1);
                }
            }
            printf("[OnlineGame|Client] Received name %hhu: %.*s!\n", namePL.player, 8, namePL.name);
            m_lobby.names[namePL.player] = std::string(namePL.name, std::min(strlen(namePL.name), 8ull));
        }
        printf("[OnlineGame|Client] Received all names!\n");
    }

    void ClientGame::close() {
        if (m_lobby.host) {
            m_lobby.host.getSocket().lock()->disconnect();
        }
        m_activeChar.reset();
    }

    void ClientGame::update() {
        auto &host = m_lobby.host;
        if (host.receive() == -1) {
            m_next = std::make_shared<padi::content::MainMenu>(
                    "../media/ui.apollo",
                    "../media/ui_sheet.png"
            );
            return;
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
            printChatMessage(std::string(payload.message, std::min(32ull, strlen(payload.message))));
        }
        takeTurn();
    }

    void ClientGame::takeTurn() {
        if (m_activeChar) {
            if (m_activeChar->controller(shared_from_this(), m_activeChar)) {
                m_activeChar.reset();
            }
        } else {
            CharacterTurnBeginPayload nextTurn(0);
            if (m_lobby.host.fetch(nextTurn)) {
                printf("[OnlineGame|Client] Character %u is starting their turn.\n", nextTurn.characterId);
                m_activeChar = m_characters.at(nextTurn.characterId);
                if (m_activeChar->entity) {
                    m_level->centerView(m_activeChar->entity->getPosition());
                    m_level->moveCursor(m_activeChar->entity->getPosition());
                }
            }
        }
    }

    void ClientGame::sendChatMessage(const std::string &msg) {
        ChatMessagePayload msgPayload;
        std::memcpy(msgPayload.message, msg.c_str(), std::min(msg.size(), 32ull));
        sf::Packet packet = PackagePayload(msgPayload);
        m_lobby.host.send(packet);
        m_uiContext.setFocusActive(false);
    }

    ClientGame::ClientGame(InOutBox &host, const std::string &name)
            : m_lobby({host}) {
        synchronize(name);
    }

}