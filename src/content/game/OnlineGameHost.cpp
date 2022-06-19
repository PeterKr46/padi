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
#include "../npc/Mob.h"

namespace padi::content {
    void HostGame::synchronizeSeed() {
        sf::Packet packet;
        GameSeedPayload gameSeedPL;
        gameSeedPL.seed = m_seed;
        PackagePayload(packet, gameSeedPL);
        printf("[OnlineGame|Server] Propagating seed!\n");
        for (auto &remote: m_lobby.remotes) {
            remote.send(packet);
        }
        printf("[OnlineGame|Server] Propagated seed %u!\n", m_seed);
    }

    void HostGame::initializeCharacters() {
        auto apollo = m_level->getApollo();
        sf::Packet packet;
        PlayerSpawnPayload playerSpawnPL;
        PlayerAssignAbilityPayload playerAbilityPL;
        std::shared_ptr<Character> player;
        LocalPlayerTurn localPlayerTurn(&m_uiContext);
        for (size_t id = 0; id < m_lobby.remotes.size() + 1; ++id) {
            playerSpawnPL.character = id;
            playerSpawnPL.pos = sf::Vector2i{int(id), 0};
            playerSpawnPL.color = sf::Color(m_rand()); // TODO rand progression!
            playerSpawnPL.color.a = 255;
            for (size_t sockId = 0; sockId < m_lobby.remotes.size(); ++sockId) {
                playerSpawnPL.local = sockId == id;
                m_lobby.remotes[sockId].send(PackagePayload(packet, playerSpawnPL));
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
            playerAbilityPL.abilityType = 0;
            for (int i = 0; i < 4; ++i) {
                playerAbilityPL.abilityProps[0] = 3;
                playerAbilityPL.abilitySlot = i;
                playerAbilityPL.character = id;
                for (auto &remote: m_lobby.remotes) {
                    remote.send(PackagePayload(packet, playerAbilityPL));
                }
                assignPlayerAbility(playerAbilityPL);
                playerAbilityPL.abilityType++;
            }

            if (id < m_lobby.remotes.size()) {
                player->controller = RemotePlayerTurn(m_lobby.remotes[id]);
            }

            auto spawnEvent = std::make_shared<padi::SpawnEvent>(player->entity);
            spawnEvent->dispatch(m_level);
            m_turnQueue.push(id);
        }
        m_characters.at(m_lobby.remotes.size())->controller = LocalPlayerTurn(&m_uiContext);

        auto mob = std::make_shared<Mob>("mob", m_level->getApollo()->lookupAnimContext("bubbleboi"), sf::Vector2i{3,3});
        auto cr = mob->asCharacter(0);
        spawnCharacter(cr);
    }

    void HostGame::synchronizeLobby(const std::string &name) {
        sf::Packet packet;
        LobbySizePayload lobbySizePL;
        PlayerNamePayload namePL;

        m_lobby.size = m_lobby.remotes.size() + 1;
        m_lobby.names.resize(m_lobby.size, "");

        // HOST     propagate lobby size
        // HOST     receive all names
        // HOST     propagate all names
        printf("[OnlineGame|Server] Propagating lobby size!\n");
        lobbySizePL.players = m_lobby.size;
        PackagePayload(packet, lobbySizePL);
        for (auto &remote: m_lobby.remotes) {
            remote.send(packet);
        }

        printf("[OnlineGame|Server] Receiving lobby names!\n");
        for (size_t id = 0; id < m_lobby.remotes.size(); ++id) {
            auto &remote = m_lobby.remotes[id];
            while (!remote.fetch(namePL)) {
                if (remote.receive() == -1) {
                    printf("[OnlineGame|Server] Lost connection!\n");
                    exit(-1);
                }
            }
            namePL.character = id;
            m_lobby.names[id] = std::string(namePL.name, std::min(strlen(namePL.name), 8ull));
            printf("[OnlineGame|Server] Received name %zu: %.*s!\n", id, 8, namePL.name);
        }
        for (size_t id = 0; id < m_lobby.remotes.size(); ++id) {

            auto &playerName = m_lobby.names[id];
            namePL.character = id;
            std::memcpy(&namePL.name, playerName.c_str(), std::min(8ull, playerName.length()));
            PackagePayload(packet, namePL);

            for (size_t clientId = 0; clientId < m_lobby.remotes.size(); ++clientId) {
                if (clientId != id) {
                    m_lobby.remotes[clientId].send(packet);
                }
            }
        }
        std::memcpy(&namePL.name, name.c_str(), std::min(8ull, name.length()));
        namePL.character = m_lobby.remotes.size();
        printf("[OnlineGame|Server] Propagating own name %i: %.*s!\n", namePL.character, 8, namePL.name);
        m_lobby.names.back() = name;
        PackagePayload(packet, namePL);
        broadcast(packet);
    }

    void HostGame::close() {
        for (auto &remote: m_lobby.remotes) {
            auto socket = remote.getSocket().lock();
            if (socket) socket->disconnect();
        }
        while (!m_turnQueue.empty()) m_turnQueue.pop();
        m_activeChar.reset();
    }

    void HostGame::update() {
        for (size_t cid = 0; cid < m_lobby.remotes.size(); ++cid) {
            auto &client = m_lobby.remotes[cid];
            if (client) {
                if (client.receive() == -1) {
                    printf("[OnlineGame|Server] Client %zull (%s) lost connection.\n", cid, m_lobby.names[cid].c_str());
                    m_lobby.remotes[cid] = InOutBox(); // TODO how do i handle this?
                    m_level->getMap()->removeEntity(m_characters[cid]->entity);
                    m_chat.ui.submit(m_characters[cid]->entity->getName() + " lost connection.");
                    if (m_activeChar == m_characters[cid]) {
                        m_activeChar.reset();
                    }
                } else {
                    if (client.has(PayloadType::ChatMessage)) {
                        ChatMessagePayload payload;
                        client.fetch(payload);
                        sf::Packet chatPacket = PackagePayload(payload);
                        broadcast(chatPacket);
                        printChatMessage(std::string(payload.message, std::min(32ull, strlen(payload.message))));
                    }
                }
            }
        }
        takeTurn();
    }

    void HostGame::advanceTurn() {
        if (!m_turnQueue.empty()) {
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
            if (m_activeChar->id < m_lobby.remotes.size()) {
                auto packet = PackagePayload(ChatMessagePayload{ChatMessage, "Your turn.\0"});
                m_lobby.remotes[m_activeChar->id].send(packet);
            } else if (m_activeChar->id == m_lobby.remotes.size() ||
                       (m_activeChar->id == 0 && m_lobby.remotes.empty())) {
                printChatMessage("Your turn.");
            }
            auto nextTurn = PackagePayload(CharacterTurnBeginPayload(m_activeChar->id));
            broadcast(nextTurn);
        }

    }

    void HostGame::takeTurn() {
        if (m_activeChar) {
            if (m_activeChar->controller(shared_from_this(), m_activeChar)) {
                advanceTurn();
            }
        } else {
            advanceTurn();
        }
    }

    void HostGame::sendChatMessage(const std::string &msg) {
        ChatMessagePayload msgPayload;
        std::memcpy(msgPayload.message, msg.c_str(), std::min(msg.size(), 32ull));
        sf::Packet packet = PackagePayload(msgPayload);
        broadcast(packet);
        printChatMessage(msg);
    }

    HostGame::HostGame(const std::vector<InOutBox> &clients, const std::string &name, size_t seed)
            : m_lobby({clients}) {
        m_seed = seed;
        m_rand = std::mt19937(seed);
        synchronize(name);
    }

    void HostGame::broadcast(sf::Packet &packet) {
        for (auto &client: m_lobby.remotes) {
            if(client) client.send(packet);
        }
    }

    uint32_t HostGame::spawnCharacter(Character & c) {
        uint32_t cid = m_characters.rbegin()->first + 1;
        auto & newChar = m_characters[cid];
        newChar = std::make_shared<Character>(Character{cid, c.entity, c.abilities, c.controller, c.alive});
        if(newChar->entity) {
            auto spawnEvent = std::make_shared<SpawnEvent>(c.entity);
            spawnEvent->dispatch(m_level);
            auto package = PackagePayload(PlayerSpawnPayload{PayloadType::CharacterSpawn, cid, c.entity->getPosition(), c.entity->getColor(),false});
            broadcast(package);
        }
        sf::Packet packet;
        for(size_t aid = 0; aid < c.abilities.size(); ++aid) {
            PlayerAssignAbilityPayload p {PayloadType::CharacterAbilityAssign, cid, uint8_t(aid),c.abilities[aid]->getAbilityType() };
            c.abilities[aid]->writeProperties(p.abilityProps, 16);
            PackagePayload(packet, p);
            broadcast(packet);
        }
        m_turnQueue.push(cid);
        return cid;
    }
}