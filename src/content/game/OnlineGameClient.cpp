//
// Created by Peter on 17/06/2022.
//

#include "OnlineGame.h"
#include "LocalPlayerTurn.h"
#include "Character.h"
#include "RemotePlayerTurn.h"
#include "../../level/SpawnEvent.h"
#include "../menu/MainMenu.h"
#include "../npc/Beacon.h"
#include "Narrator.h"

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
        m_characters.clear();
        m_activeChar.reset();

        CharacterSpawnPayload characterPL;
        EntitySpawnPayload entityPL;
        std::shared_ptr<Character> chr;
        for (size_t id = 0; id < m_lobby.size; ++id) {
            if (!m_lobby.host.fetchBlocking(characterPL)) {
                exit(-1);
            }
            if (!m_lobby.host.fetchBlocking(entityPL)) {
                exit(-1);
            }
            spawnNewCharacter(characterPL);
            spawnNewEntity(entityPL);
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
        if (!m_lobby.host.fetchBlocking(lobbySizePL)) {
            exit(-1);
        }
        printf("[OnlineGame|Client] Received lobby size: %hhu!\n", lobbySizePL.numPlayers);
        m_lobby.size = lobbySizePL.numPlayers;
        m_lobby.names.resize(m_lobby.size, "");
        printf("[OnlineGame|Client] Sending own name!\n");
        std::memcpy(&namePL.name, ownName.c_str(), std::min<size_t>(8ull, ownName.length()));
        PackagePayload(packet, namePL);
        m_lobby.host.send(packet);
        printf("[OnlineGame|Client] Sent own name!\n");
        for (size_t id = 0; id < m_lobby.names.size(); ++id) {
            if (!m_lobby.host.fetchBlocking(namePL)) {
                exit(-1);
            }
            printf("[OnlineGame|Client] Received name %u: %.*s!\n", namePL.cid, 8, namePL.name);
            m_lobby.names[namePL.cid] = std::string(namePL.name, std::min<size_t>(strlen(namePL.name), 8ull));
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
        // TODO clean up this mess...
        while (host.has(PayloadType::CharacterSpawn)) {
            CharacterSpawnPayload characterSpawn;
            host.fetch(characterSpawn);
            spawnNewCharacter(characterSpawn);
        }
        while (host.has(PayloadType::EntitySpawn)) {
            EntitySpawnPayload entitySpawn;
            host.fetch(entitySpawn);
            if (m_characters.find(entitySpawn.cid) == m_characters.end()) {
                printf("[OnlineGame|Client] Received invalid Entity spawn data! (Unknown character id)\n");
            } else {
                spawnNewEntity(entitySpawn);
            }
        }
        while (host.has(PayloadType::InitHP)) {
            InitHPPayload payload;
            host.fetch(payload);
            auto chr = m_characters[payload.cid];
            auto entity = chr->entity;
            entity->initHPBar(payload.maxHP,
                             m_level->getApollo()->lookupAnimContext("hp_bars"),
                             payload.color);
            if(!chr->awake) {
                entity->getHPBar().lock()->asleep = true;
            }
        }
        while (host.has(PayloadType::CharacterAbilityAssign)) {
            CharacterAbilityAssignPayload abilityPayload;
            host.fetch(abilityPayload);
            assignPlayerAbility(abilityPayload);
        }
        while (host.has(PayloadType::ChatMessage)) {
            ChatMessagePayload payload;
            host.fetch(payload);
            if(payload.cid == -1) {
                printChatMessage(std::string(payload.message, std::min(sizeof(payload.message), strlen(payload.message))), true);
            } else {
                printChatMessage(m_lobby.names[payload.cid] + ": " + std::string(payload.message, std::min(sizeof(payload.message), strlen(payload.message))),true);
            }
        }
        while(host.has(PayloadType::PlayerDespawn)) {
            PlayerDespawnPayload payload;
            host.fetch(payload);
            auto & chr = m_characters[payload.cid];
            chr->alive = false;
            m_level->getMap()->removeEntity(chr->entity);
        }
        while (host.has(PayloadType::EventSpawn)) {
            EventSpawnPayload payload;
            host.fetch(payload);
            spawnEvent(payload.pos, payload);
        }
        while(host.has(PayloadType::EventDespawn)) {
            EventDespawnPayload payload;
            host.fetch(payload);
            despawnEvent(payload.pos);
        }
        while(host.has(PayloadType::Narration)) {
            NarratorPayload payload;
            host.fetch(payload);
            if(m_narrator) {
                m_narrator->queue(payload.event);
            }
        }
        takeTurn();
    }

    void ClientGame::takeTurn() {
        if (m_activeChar) {
            if (m_activeChar->controller(shared_from_this(), m_activeChar)) {
                m_activeChar.reset();
            }
        } else {
            auto &host = m_lobby.host;
            if (host.has(CharacterTurnBegin)) {
                CharacterTurnBeginPayload nextTurn(0);
                host.fetch(nextTurn);
                printf("[OnlineGame|Client] Character %u is starting their turn.\n", nextTurn.cid);
                m_activeChar = m_characters.at(nextTurn.cid);
                if (m_activeChar->entity) {
                    m_level->centerView(m_activeChar->entity->getPosition());
                    m_level->moveCursor(m_activeChar->entity->getPosition());
                    if(m_activeChar->entity->hasHPBar()) {
                        m_activeChar->entity->getHPBar().lock()->asleep = false;
                    }
                }
            } else if (host.has(PayloadType::NextLevel)) {
                synchronize(m_lobby.names[m_localChar]); // TODO
            }
        }
    }

    void ClientGame::sendChatMessage(const std::string &msg) {
        if(msg.empty()) return;
        sf::Packet packet = PackagePayload(ChatMessagePayload(0, msg.c_str()));
        m_lobby.host.send(packet);
        m_uiContext.setFocusActive(false);
    }

    ClientGame::ClientGame(InOutBox &host, const std::string &name)
            : m_lobby({host}){
        synchronize(name);
        m_narrator = std::make_shared<RemoteNarrator>();
    }

    void ClientGame::broadcast(sf::Packet &packet) {
        m_lobby.host.send(packet);
    }

    void ClientGame::spawnNewEntity(EntitySpawnPayload payload) {
        auto charIter = m_characters.find(payload.cid);
        if(charIter != m_characters.end()) {
            auto & chr = charIter->second;
            if(payload.entitytype & BEACON) {
                chr->entity = std::make_shared<Beacon>("EndGate", m_level->getApollo()->lookupAnimContext(payload.animations),
                                                       payload.pos);
            } else {
                // GENERIC LIVING ENTITY
                chr->entity = std::make_shared<padi::LivingEntity>(
                        payload.cid < m_lobby.size ? m_lobby.names[payload.cid] : "_LivingEntity",
                        m_level->getApollo()->lookupAnimContext(payload.animations),
                        payload.pos, payload.entitytype);
            }
            chr->entity->setColor(payload.color);
            auto spawnEvent = std::make_shared<SpawnEvent>(chr->entity);
            spawnEvent->dispatch(m_level);
        } else if(payload.cid == ~0u) {
            printf("[OnlineGame|Client] Spawning detached Entity!");
            auto entity = std::make_shared<padi::LivingEntity>(
                    "_LivingEntity",
                    m_level->getApollo()->lookupAnimContext(payload.animations),
                    payload.pos, payload.entitytype);
            entity->setColor(payload.color);
            auto spawnEvent = std::make_shared<SpawnEvent>(entity);
            spawnEvent->dispatch(m_level);
        }
    }


    void ClientGame::spawnNewCharacter(CharacterSpawnPayload payload) {
        auto &newChar = m_characters[payload.cid];
        newChar = std::make_shared<Character>(Character{payload.cid});
        newChar->awake = !payload.asleep;
        if (payload.controller == CharacterSpawnPayload::LocalPlayer) {
            m_localChar = payload.cid;
            newChar->controller = LocalPlayerTurn(&m_uiContext);
        } else {
            newChar->controller = RemotePlayerTurn(m_lobby.host, false);
        }
    }

    void ClientGame::broadcast(sf::Packet &packet, const uint32_t *ignore, uint32_t num_ignored) {
        /**
         * A client will only ever send to the host - the host cannot be ignored.
         */
        uint32_t cid = m_lobby.size - 1;
        bool ignored = false;
        for(size_t j = 0; j < num_ignored; ++j) ignored |= (ignore[j] == cid);
        if (!ignored)
            m_lobby.host.send(packet);
    }

}