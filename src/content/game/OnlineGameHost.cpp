//
// Created by Peter on 17/06/2022.
//

#include "OnlineGame.h"

#include <climits>

#include "LocalPlayerTurn.h"
#include "../abilities/Abilities.h"
#include "RemotePlayerTurn.h"
#include "../../level/SpawnEvent.h"
#include "Character.h"
#include "../npc/ExplosiveMob.h"
#include "../menu/MainMenu.h"
#include "../npc/SlugMob.h"
#include "../npc/Beacon.h"
#include "Narrator.h"
#include "../../level/LevelGenerator.h"
#include "../npc/Thirdman.h"

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
        std::vector<Character> savedCharacters;
        auto iter = m_characters.begin();
        while(iter != m_characters.end()) {
            if(iter->first < m_lobby.size) {
                savedCharacters.emplace_back(* iter->second);
            }
            iter = m_characters.erase(iter);
        }
        while(!m_turnQueue.empty()) m_turnQueue.pop();

        auto apollo = m_level->getApollo();
        auto map = m_level->getMap();

        Character playerCharacter;
        sf::Color playerColor;
        sf::Vector2i nextPlayerPos;
        uint8_t nextPlayerType;
        for (size_t id = 0; id < m_lobby.size; ++id) {
            m_level->popSpawnPosition(nextPlayerPos, nextPlayerType);
            if (savedCharacters.size() > id) {
                playerCharacter.entity = savedCharacters[id].entity;
                playerCharacter.entity->switchApollo(apollo);
                //playerCharacter.entity->getHPBar().lock()->setHP(INT_MAX);

                // A really ugly hack to reposition...
                map->moveEntity(playerCharacter.entity, nextPlayerPos);

                playerCharacter.abilities = savedCharacters[id].abilities;
                playerCharacter.alive = true;
            } else {
                playerCharacter.entity = std::make_shared<padi::LivingEntity>(
                        m_lobby.names[id],
                        apollo->lookupAnimContext("cube"),
                        nextPlayerPos,
                        PLAYER
                );
                playerColor = sf::Color(hsv(int(hash_c_string(m_lobby.names[id].c_str(), m_lobby.names[id].length())), 1.f, 1.f));
                playerCharacter.entity->setColor(playerColor);
                playerCharacter.entity->initHPBar(2, apollo->lookupAnimContext("hp_bars"));

                playerCharacter.abilities = {
                        std::make_shared<Peep>(playerCharacter.entity),
                        std::make_shared<Walk>(playerCharacter.entity, 6),
                        std::make_shared<Dash>(playerCharacter.entity, 3, Walk::Walkable{100}),
                        //std::make_shared<Lighten>(playerCharacter.entity)
                };
            }
            spawnCharacter(playerCharacter, id);
        }

        {
            auto beacon = std::make_shared<Beacon>(
                    "gate",
                    m_level->getApollo()->lookupAnimContext("gate"),
                    sf::Vector2i{0, 0}
            );
            size_t slain = 0;
            for (int cid = 0; cid < m_lobby.size; cid++) {
                slain += m_characters.at(cid)->entity->enemiesSlain;
            }
            beacon->m_requiredKills = slain + std::log2(m_stage + 1.f) * 3 * m_lobby.size;
            if (m_seed == LevelGenerator::TutorialSeed) {
                beacon->m_requiredKills = 2;
            }
            auto cr = beacon->asCharacter();
            m_turnQueue.push(spawnCharacter(cr, ~0u, false));
        }

        sf::Vector2i nextMobPos;
        uint8_t nextMobType;
        while(m_level->popSpawnPosition(nextMobPos, nextMobType)) {
            bool startAwake = false; //nextPlayerType & 1u
            if(nextMobType < 128){
                auto mob = std::make_shared<ExplosiveMob>("mob", m_level->getApollo()->lookupAnimContext("bubbleboi"),
                                                          nextMobPos);
                mob->initHPBar(1, m_level->getApollo()->lookupAnimContext("hp_bars"), sf::Color::White);
                mob->getHPBar().lock()->asleep = !mob->asCharacter(startAwake).awake;
                map->moveEntity(mob, nextMobPos);
                spawnCharacter(mob->asCharacter(startAwake), ~0u);
            } else if(nextMobType < 224) {
                auto mob = std::make_shared<SlugMob>("mob", m_level->getApollo()->lookupAnimContext("tetrahedron"),
                                                     nextMobPos);
                mob->initHPBar(2, m_level->getApollo()->lookupAnimContext("hp_bars"), sf::Color::White);
                mob->getHPBar().lock()->asleep = !mob->asCharacter(true).awake;
                spawnCharacter(mob->asCharacter(startAwake), ~0u);
            } else {
                auto mob = std::make_shared<Thirdman>("volcano", m_level->getApollo()->lookupAnimContext("volcano"),
                                                     nextMobPos);
                mob->initHPBar(3, m_level->getApollo()->lookupAnimContext("hp_bars"), sf::Color::White);
                mob->getHPBar().lock()->asleep = !startAwake;
                spawnCharacter(mob->asCharacter(startAwake), ~0u, false);
            }
        }
        for(auto & [id, chr] : m_characters) {
            if(chr->entity) map->removeEntity(chr->entity);
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
        broadcast(packet);

        printf("[OnlineGame|Server] Receiving lobby names!\n");
        for (size_t id = 0; id < m_lobby.remotes.size(); ++id) {
            auto &remote = m_lobby.remotes[id];
            if (!remote.fetchBlocking(namePL)) {
                exit(-1);
            }
            namePL.cid = id;
            m_lobby.names[id] = std::string(namePL.name, std::min<size_t>(strlen(namePL.name), 8ull));
            printf("[OnlineGame|Server] Received name %zu: %.*s!\n", id, 8, namePL.name);
        }
        for (size_t id = 0; id < m_lobby.remotes.size(); ++id) {

            auto &playerName = m_lobby.names[id];
            namePL.cid = id;
            std::memcpy(&namePL.name, playerName.c_str(), std::min<size_t>(8ull, playerName.length()));
            PackagePayload(packet, namePL);

            broadcast(packet);
        }
        std::memcpy(&namePL.name, name.c_str(), std::min<size_t>(8ull, name.length()));
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
                        client.fetch(payload);
                        sendChatGeneric(std::string(payload.message, std::min(sizeof(payload.message), strlen(payload.message))), cid);
                    }
                }
            }
        }
        takeTurn();
        fflush(stdout);
    }

    void HostGame::advanceTurn() {
        if(m_activeChar) {
            printf("[OnlineGame|Host] Character %i completed their turn.\n", m_activeChar->id);
            if(m_activeChar->id == m_lobby.size - 1) {
                sendChatGeneric("The darkness moves.");
            }
            // TODO this should probably not be right here...
            detectEventInteraction();
            m_activeChar.reset();
            m_roundCooldown.restart();
        }

        if(!(*m_narrator)(shared_from_this(), nullptr)) {
            return;
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
                        auto packet = PackagePayload(PlayerDespawnPayload(chr->id));
                        broadcast(packet);
                        if(chr->entity->getType() & EntityType::PLAYER) {
                            sendChatGeneric(m_lobby.names[chr->id] + " died.");
                        } else if (m_seed == LevelGenerator::TutorialSeed || m_rand() % 256 > 128 ) {
                            spawnDropEvent(chr->entity->getPosition());
                            if (m_seed == LevelGenerator::TutorialSeed) {
                               m_narrator->queueCenter(chr->entity->getPosition());
                               m_narrator->queueText("They dropped something!");
                               m_narrator->queueConfirm();
                            }
                        }
                    }
                }
                if (chr->alive) {
                    m_turnQueue.push(cid);
                }
            }
        }
        {
            m_activeChar = m_characters.at(m_turnQueue.front());
            m_turnQueue.pop();

            if (!m_activeChar->awake && m_activeChar->wakeupRange > 0) {
                auto activeEntity = m_activeChar->entity;
                if (activeEntity) {
                    auto pos = activeEntity->getPosition();
                    for (int i = 0; i < m_lobby.size; ++i) {
                        auto player = m_characters[i]->entity;
                        if (player) {
                            auto delta = player->getPosition() - pos;
                            if (std::abs(delta.x) + std::abs(delta.y) < m_activeChar->wakeupRange) {
                                m_activeChar->awake = true;
                                if (activeEntity->hasHPBar()) {
                                    activeEntity->getHPBar().lock()->asleep = false;
                                }
                                break;
                            }
                        }
                    }
                }
                if (!m_activeChar->awake) {
                    m_activeChar.reset();
                    return;
                }
            }
            if (m_activeChar->entity) {
                m_level->centerView(m_activeChar->entity->getPosition());
                m_level->moveCursor(m_activeChar->entity->getPosition());
                // Dead characters don't get to take their turns :c
                if (m_activeChar->entity->hasHPBar()) {
                    if (m_activeChar->entity->getHPBar().lock()->getHP() == 0) {
                        m_activeChar.reset();
                        return;
                    }
                }
            }
            printf("[OnlineGame|Server] Character %i is starting their turn.\n", m_activeChar->id);
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
        msgPayload.cid = ~0u;
        if(msg == "exit") {
            m_next = std::make_shared<MainMenu>(
                    "../media/ui.apollo",
                    "../media/ui_sheet.png",
                    m_lobby.names[m_lobby.size - 1].c_str()
            );
        } else if(msg == "restart" || msg == "autowin") {
            signalLevelAdvance();
            sendChatGeneric("Restarting after this round.");
            return;
        } else if(msg == "give raze" && from < m_lobby.size) {
            CharacterAbilityAssignPayload p{PayloadType::CharacterAbilityAssign, from, 3, AbilityType::Raze };
            assignPlayerAbility(p);
            auto pak = PackagePayload(p);
            broadcast(pak);
            return;
        } else if(msg == "give tp" && from < m_lobby.size) {
            CharacterAbilityAssignPayload p{PayloadType::CharacterAbilityAssign, from, 3, AbilityType::Teleport };
            assignPlayerAbility(p);
            auto pak = PackagePayload(p);
            broadcast(pak);
            return;
        } else if(msg == "give wildfire" && from < m_lobby.size) {
            CharacterAbilityAssignPayload p{PayloadType::CharacterAbilityAssign, from, 3, AbilityType::Wildfire };
            Wildfire(nullptr, 14).writeProperties(p.abilityProps, 16);
            auto pak = PackagePayload(p);
            broadcast(pak);
            assignPlayerAbility(p);
            return;
        } else if(msg[0] == '?') {
            if(std::strstr(msg.c_str(), "?BLESS") || std::strstr(msg.c_str(), "?BLESSED")) {
                sendChatGeneric("BLESSED Tiles are too LIGHT for DARK Creatures to enter. They can generally not be DARKened.");
                return;
            }
            else if(std::strstr(msg.c_str(), "?CURSE") ||std::strstr(msg.c_str(), "?CURSED")) {
                sendChatGeneric("CURSED Tiles are too DARK for LIGHT Creatures (you) to enter. They can generally not be LIGHTened.");
                return;
            } else {
                sendChatGeneric("exit ?BLESS ?CURSE restart/autowin");
                return;
            }
        }
        std::string buf;
        if(from < m_lobby.size) {
            buf = m_lobby.names[from] + ": " + msg;
        }
        else {
            buf = msg;
        }

        std::string_view view = buf;
        do {
            std::memcpy(msgPayload.message, view.data(), sizeof(char) * std::min(view.length(), sizeof(msgPayload.message)));
            if(view.length() < sizeof(msgPayload.message) / sizeof(char)) {
                msgPayload.message[view.length()] = '\0';
            }
            sf::Packet packet = PackagePayload(msgPayload);
            broadcast(packet);
            printChatMessage(std::string(view.substr(0,std::min(view.length(), sizeof(msgPayload.message)))), msgPayload.cid != ~0u);

            view.remove_prefix(std::min(view.length(), sizeof(msgPayload.message)));
        } while(view.length() > 0);
    }

    void HostGame::sendChatMessage(const std::string &msg) {
        if(msg.empty()) return;
        sendChatGeneric(msg, m_lobby.size - 1);
    }

    HostGame::HostGame(const std::vector<InOutBox> &clients, const std::string &name, size_t seed)
            : m_lobby({clients}) {
        m_seed = seed;
        m_rand = std::mt19937(seed);
        m_narrator = seed == LevelGenerator::TutorialSeed ? std::make_shared<Tutorial>() : std::make_shared<Narrator>();
        synchronize(name);
    }

    void HostGame::broadcast(sf::Packet &packet) {
        for (auto &client: m_lobby.remotes) {
            if (client) client.send(packet);
        }
    }

    uint32_t HostGame::spawnCharacter(Character const& c, uint32_t owner, bool spawnAnimation) {
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
                        c.alive,
                        c.awake,
                        c.wakeupRange
                });

        if(owner < m_lobby.size - 1) {
            newChar->controller = RemotePlayerTurn(m_lobby.remotes[owner], true, cid);
        } else if (owner == m_lobby.size - 1) {
            newChar->controller = LocalPlayerTurn(&m_uiContext);
        }

        sf::Packet packet;

        // Let everyone know about the character.
        for (int32_t i = 0; i < m_lobby.remotes.size(); ++i) {
            auto &remote = m_lobby.remotes[i];
            auto payload = CharacterSpawnPayload{PayloadType::CharacterSpawn, cid};
            payload.controller = (i == owner) ? CharacterSpawnPayload::LocalPlayer : CharacterSpawnPayload::RemotePlayer;
            payload.asleep = !newChar->awake;
            PackagePayload(packet, payload);
            remote.send(packet);
        }

        // If the character has an entity, let everyone know about it.
        if (newChar->entity) {
            EntitySpawnPayload payload;
            payload.cid = cid;
            payload.entitytype = c.entity->getType();
            payload.pos = c.entity->getPosition();
            payload.color = c.entity->getColor();
            payload.spawnAnimation = spawnAnimation;
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
            auto spawnEvent = std::make_shared<SpawnEvent>(c.entity, spawnAnimation);
            spawnEvent->dispatch(m_level);
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

    void HostGame::broadcast(sf::Packet &packet, const uint32_t *ignore, uint32_t num_ignored) {
        for (size_t cid = 0; cid < m_lobby.remotes.size(); ++cid) {
            bool ignored = false;
            for(size_t j = 0; j < num_ignored; ++j) ignored |= (ignore[j] == cid);
            if(!ignored) m_lobby.remotes[cid].send(packet);
        }
    }

    void HostGame::endOfRound() {
        printf("[OnlineGame|Server] Round ended.\n");
        if(m_levelComplete) {
            auto packet = PackagePayload(NextLevelPayload{});
            broadcast(packet);
            synchronize(m_lobby.names[m_lobby.size-1]);
            m_levelComplete = false;
        }
    }

    size_t HostGame::getLobbySize() const {
        return m_lobby.size;
    }

    void HostGame::signalLevelAdvance() {
        m_levelComplete = true;
        m_seed = (m_seed * 456123) % 4574567;
    }

    void HostGame::spawnDropEvent(const sf::Vector2i &pos) {
        EventSpawnPayload payload;
        payload.pos = pos;
        static const uint8_t possibleDrops[]{AbilityType::Raze, AbilityType::Lighten, AbilityType::Teleport, AbilityType::Wildfire};
        payload.abilityType = possibleDrops[m_rand() % sizeof(possibleDrops)];
        auto packet = PackagePayload(payload);
        broadcast(packet);
        spawnEvent(pos, payload);
    }

    void HostGame::detectEventInteraction() {
        if (m_activeChar && m_activeChar->id < m_lobby.size && m_activeChar->alive) {
            auto pos = m_activeChar->entity->getPosition();
            auto event = m_events.find(pos);
            if (event != m_events.end()) {
                printf("%s is interacting with an event at %i, %i.\n", m_lobby.names[m_activeChar->id].c_str(), pos.x, pos.y);
                auto & data = event->second.second;
                {
                    CharacterAbilityAssignPayload payload;
                    std::memcpy(payload.abilityProps, data.abilityProps, sizeof(payload.abilityProps));
                    payload.abilityType = data.abilityType;
                    payload.cid = m_activeChar->id;
                    payload.abilitySlot = 3;
                    assignPlayerAbility(payload);
                    auto packet = PackagePayload(payload);
                    broadcast(packet);
                }
                {
                    EventDespawnPayload payload;
                    payload.pos = pos;
                    auto packet = PackagePayload(payload);
                    broadcast(packet);
                    despawnEvent(pos);
                }
                {
                    if(m_activeChar->id < m_lobby.size - 1) {
                        auto outbox = m_lobby.remotes[m_activeChar->id];
                        sf::Packet packet;

                        NarratorPayload payload{};

                        const char* msg = "You found a new ability!";
                        payload.event.type = NarratorEvent::ShowText;
                        payload.event.data.showText.center = true;
                        std::strncpy(payload.event.data.showText.text, msg, std::min(strlen(msg), sizeof(payload.event.data.showText.text) / sizeof(char) - 1));

                        packet = PackagePayload(payload);
                        outbox.send(packet);

                        payload.event.type = NarratorEvent::ShowSprite;
                        payload.event.data.showSprite.pos = {224,128};
                        auto id2 = m_activeChar->abilities[3]->getIconId().c_str();
                        std::strncpy(payload.event.data.showSprite.id, id2, std::min(strlen(id2), sizeof(payload.event.data.showSprite.id) / sizeof(char) - 1));
                        packet = PackagePayload(payload);
                        outbox.send(packet);

                        payload.event.type = NarratorEvent::Confirm;
                        packet = PackagePayload(payload);
                        outbox.send(packet);
                    } else {
                        m_narrator->queueText("You found a new ability!");
                        m_narrator->queueSprite(m_activeChar->abilities[3]->getIconId().c_str(), {224,128});
                        m_narrator->queueConfirm();
                    }
                }
            }
        }
    }

}