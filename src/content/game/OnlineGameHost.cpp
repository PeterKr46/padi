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
#include "../npc/ExplosiveMob.h"
#include "../menu/MainMenu.h"
#include "../npc/SlugMob.h"
#include "../npc/EndGate.h"

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

        Character playerCharacter;
        sf::Color playerColor;
        int offset = 1;
        for (size_t id = 0; id < m_lobby.size; ++id) {
            if(id - 4*(offset-1) >= 4) offset++;
            playerCharacter.entity = std::make_shared<padi::LivingEntity>(
                    m_lobby.names[id],
                    apollo->lookupAnimContext("cube"),
                    AllDirections[id%4] * offset
            );
            playerColor = sf::Color(std::hash<std::string>()(m_lobby.names[id]));
            playerColor.a = 0xFF;
            playerCharacter.entity->setColor(playerColor);
            playerCharacter.entity->initHPBar(2, apollo->lookupAnimContext("hp_bars"));
            playerCharacter.abilities = {
                    std::make_shared<Peep>(playerCharacter.entity),
                    std::make_shared<Walk>(playerCharacter.entity, 5),
                    std::make_shared<Dash>(playerCharacter.entity, 3, Walk::Walkable{100})
            };
            spawnCharacter(playerCharacter, id);
        }
        for (int i = 0; i < m_lobby.size * 2; ++i) {
            auto refPos = m_characters[i % m_lobby.size]->entity->getPosition();
            std::shared_ptr<Tile> target = nullptr;
            while(!target) {
                target = m_level->getMap()->getTile(refPos + AllDirections[m_rand() % 4] + AllDirections[m_rand() % 4] +AllDirections[m_rand() % 4] + AllDirections[m_rand() % 4]);
                if(target && (!target->m_walkable || m_level->getMap()->hasEntities(target->getPosition(), LivingEntity::EntityType))) {
                    target.reset();
                }
            }
            auto mob = std::make_shared<ExplosiveMob>("mob", m_level->getApollo()->lookupAnimContext("bubbleboi"),
                                                      target->getPosition());
            mob->initHPBar(1, m_level->getApollo()->lookupAnimContext("hp_bars"), sf::Color::White);

            auto cr = mob->asCharacter(0);
            spawnCharacter(cr, m_lobby.size - 1);
        }
        {
            auto mob = std::make_shared<SlugMob>("mob", m_level->getApollo()->lookupAnimContext("tetrahedron"),
                                                 sf::Vector2i{2, 2});
            mob->initHPBar(5, m_level->getApollo()->lookupAnimContext("hp_bars"), sf::Color::White);

            auto cr = mob->asCharacter(0);
            spawnCharacter(cr, m_lobby.size - 1);
        }
        {
            auto mob = std::make_shared<EndGate>("gate", m_level->getApollo()->lookupAnimContext("gate"),
                                                 sf::Vector2i{-3, -3});
            mob->m_requiredKills = m_lobby.size * 3;
            auto cr = mob->asCharacter(0);
            m_turnQueue.push(spawnCharacter(cr, m_lobby.size - 1));
        }
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
        lobbySizePL.numPlayers = m_lobby.size;
        PackagePayload(packet, lobbySizePL);
        for (auto &remote: m_lobby.remotes) {
            remote.send(packet);
        }

        printf("[OnlineGame|Server] Receiving lobby names!\n");
        for (size_t id = 0; id < m_lobby.remotes.size(); ++id) {
            auto &remote = m_lobby.remotes[id];
            if (!remote.fetchBlocking(namePL)) {
                exit(-1);
            }
            namePL.cid = id;
            m_lobby.names[id] = std::string(namePL.name, std::min(strlen(namePL.name), 8ull));
            printf("[OnlineGame|Server] Received name %zu: %.*s!\n", id, 8, namePL.name);
        }
        for (size_t id = 0; id < m_lobby.remotes.size(); ++id) {

            auto &playerName = m_lobby.names[id];
            namePL.cid = id;
            std::memcpy(&namePL.name, playerName.c_str(), std::min(8ull, playerName.length()));
            PackagePayload(packet, namePL);

            broadcast(packet);
        }
        std::memcpy(&namePL.name, name.c_str(), std::min(8ull, name.length()));
        namePL.cid = m_lobby.remotes.size();
        printf("[OnlineGame|Server] Propagating own name %i: %.*s!\n", namePL.cid, 8, namePL.name);
        m_lobby.names.back() = name;
        PackagePayload(packet, namePL);
        broadcast(packet);
    }

    void HostGame::close() {
        for (auto &remote: m_lobby.remotes) {
            auto socket = remote.getSocket().lock();
            if (socket) socket->disconnect();
        }
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
                        payload.cid = cid;
                        client.fetch(payload);
                        sf::Packet chatPacket = PackagePayload(payload);
                        broadcast(chatPacket);
                        printChatMessage(m_lobby.names[cid] + ": " + std::string(payload.message, std::min(sizeof(payload.message), strlen(payload.message))));
                    }
                }
            }
        }
        takeTurn();
    }

    void HostGame::advanceTurn() {
        if(m_activeChar) {
            if(m_activeChar->id == m_lobby.size - 1) {
                sendChatGeneric("The darkness moves.");
            }
            m_activeChar.reset();
            m_roundCooldown.restart();
        }

        if(m_turnQueue.empty()) {
            if (m_roundCooldown.getElapsedTime().asMilliseconds() < 1000) {
                return;
            }
            // End of Round fun!
            endOfRound();

            // Construct queue for next round.
            for( auto & [cid, chr] : m_characters) {
                // Entities that have an HP Bar which dropped to zero die at the end of the round.
                if (chr->alive && chr->entity && chr->entity->hasHPBar()) {
                    chr->alive &= (chr->entity->getHPBar().lock()->getHP() > 0);
                    if (!chr->alive) {
                        m_level->getMap()->removeEntity(chr->entity);
                        if(chr->id < m_lobby.size) sendChatGeneric(m_lobby.names[chr->id] + " died.");
                        auto packet = PackagePayload(PlayerDespawnPayload(chr->id));
                        broadcast(packet);
                    }
                }
                if (chr->alive) {
                    m_turnQueue.push(cid);
                }
            }
        } else {
            printf("[OnlineGame|Server] Character %i is starting their turn.\n", m_turnQueue.front());
            m_activeChar = m_characters.at(m_turnQueue.front());
            m_turnQueue.pop();

            if (m_activeChar->entity) {
                m_level->centerView(m_activeChar->entity->getPosition());
                m_level->moveCursor(m_activeChar->entity->getPosition());

                // Dead characters don't get to take their turns :c
                if(m_activeChar->entity->hasHPBar()) {
                    if(m_activeChar->entity->getHPBar().lock()->getHP() == 0) {
                        m_activeChar.reset();
                        return;
                    }
                }
            }
            if (m_activeChar->id < m_lobby.remotes.size()) {
                auto packet = PackagePayload(ChatMessagePayload(-1, "Your turn."));
                m_lobby.remotes[m_activeChar->id].send(packet);
            } else if (m_activeChar->id == m_lobby.remotes.size() ||
                       (m_activeChar->id == 0 && m_lobby.remotes.empty())) {
                printChatMessage("Your turn.", true);
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

    void HostGame::sendChatGeneric(const std::string &msg, uint32_t from) {
        if(msg.empty()) return;

        ChatMessagePayload msgPayload;
        if(msg == "exit") {
            m_next = std::make_shared<MainMenu>(
                    "../media/ui.apollo",
                    "../media/ui_sheet.png"
            );
        }
        msgPayload.cid = from;
        std::memcpy(msgPayload.message, msg.c_str(), std::min(msg.size(), sizeof(msgPayload.message)));
        sf::Packet packet = PackagePayload(msgPayload);
        broadcast(packet);
        if(from < m_lobby.size) {
            printChatMessage(m_lobby.names[m_lobby.size - 1] + ": " + msg.substr(0, sizeof(msgPayload.message)), true);
        } else {
            printChatMessage(msg, true);
        }
    }

    void HostGame::sendChatMessage(const std::string &msg) {
        if(msg.empty()) return;
        sendChatGeneric(msg, m_lobby.size - 1);
    }

    HostGame::HostGame(const std::vector<InOutBox> &clients, const std::string &name, size_t seed)
            : m_lobby({clients}) {
        m_seed = seed;
        m_rand = std::mt19937(seed);
        synchronize(name);
    }

    void HostGame::broadcast(sf::Packet &packet) {
        for (auto &client: m_lobby.remotes) {
            if (client) client.send(packet);
        }
    }

    uint32_t HostGame::spawnCharacter(Character const& c, uint32_t owner) {
        // Determine final Character ID
        uint32_t cid = m_characters.empty() ? 0 : m_characters.rbegin()->first + 1;

        // Copy into Character map at corresponding position
        auto &newChar = m_characters[cid];

        newChar = std::make_shared<Character>(
                Character{
                        cid,
                        c.entity,
                        c.abilities,
                        c.controller,
                        c.alive
                });

        if(cid < m_lobby.size - 1) {
            newChar->controller = RemotePlayerTurn(m_lobby.remotes[owner], true, cid);
        } else if (cid == m_lobby.size - 1) {
            newChar->controller = LocalPlayerTurn(&m_uiContext);
        }

        sf::Packet packet;

        // Let everyone know about the character.
        for (int32_t i = 0; i < m_lobby.remotes.size(); ++i) {
            auto &remote = m_lobby.remotes[i];
            auto payload = CharacterSpawnPayload{PayloadType::CharacterSpawn, cid};
            payload.controller = (i == owner) ? CharacterSpawnPayload::LocalPlayer : CharacterSpawnPayload::RemotePlayer;
            PackagePayload(packet, payload);
            remote.send(packet);
        }

        // If the character has an entity, let everyone know about it.
        if (newChar->entity) {
            auto spawnEvent = std::make_shared<SpawnEvent>(c.entity);
            spawnEvent->dispatch(m_level);
            EntitySpawnPayload payload;
            payload.cid = cid;
            payload.pos = c.entity->getPosition();
            payload.color = c.entity->getColor();
            auto & animSet = c.entity->getAnimationSet()->getName();
            std::memcpy(payload.animations, animSet.data(), animSet.length());

            PackagePayload(packet, payload);
            broadcast(packet);
            if(newChar->entity->hasHPBar()) {
                auto hpBar = newChar->entity->getHPBar().lock();
                InitHPPayload hpPayload;
                hpPayload.cid = cid;
                hpPayload.maxHP = uint32_t(hpBar->getMaxHP());
                hpPayload.color = hpBar->m_overrideColor;
                PackagePayload(packet, hpPayload);
                broadcast(packet);
            }
        }
        // Let everyone know about the Character's abilities.
        for (size_t aid = 0; aid < c.abilities.size(); ++aid) {
            CharacterAbilityAssignPayload p{PayloadType::CharacterAbilityAssign, cid, uint8_t(aid),
                                            c.abilities[aid]->getAbilityType()};
            c.abilities[aid]->writeProperties(p.abilityProps, 16);
            PackagePayload(packet, p);
            broadcast(packet);
        }
        return cid;
    }

    void HostGame::advanceLevel() {
        m_activeChar.reset();
        while(!m_turnQueue.empty()) m_turnQueue.pop();

        sf::Packet packet;

        {
            auto nextLevel = NextLevelPayload{};
            broadcast(PackagePayload(packet,nextLevel));
        }
        for(size_t i = 0; i < m_lobby.size; ++i) {
            m_turnQueue.push(i);
        }
    }

    void HostGame::broadcast(sf::Packet &packet, const uint32_t *ignore, uint32_t num_ignored) {
        for (size_t cid = 0; cid < m_lobby.remotes.size(); ++cid) {
            bool ignored = false;
            for(size_t j = 0; j < num_ignored; ++j) ignored |= (ignore[j] == cid);
            if(!ignored) m_lobby.remotes[cid].send(packet);
        }
    }

    void HostGame::endOfRound() {
        printf("[OnlineGame|Server] Round ended.\n");
    }

    size_t HostGame::getLobbySize() const {
        return m_lobby.size;
    }
}