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

    OnlineGame::OnlineGame(std::vector<std::shared_ptr<sf::TcpSocket>> sockets, bool hosting, std::string const &name,
                           uint32_t seed)
            : m_lobby({hosting, std::move(sockets)}), m_seed(seed), m_rand(seed) {
        // Initialization is blocking.
        for(auto & socket : m_lobby.sockets) {
            socket->setBlocking(true);
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
        initializePlayerCharacters();
        // Everything at runtime is non-blocking.
        for(auto & socket : m_lobby.sockets) {
            socket->setBlocking(false);
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
            if (m_activeChar->controller(m_level, m_activeChar)) {
                if (m_activeChar->alive) {
                    m_characters.push(m_activeChar);
                }
                m_activeChar = m_characters.front();
                m_characters.pop();
                if (m_activeChar->entity) {
                    m_level->centerView(m_activeChar->entity->getPosition());
                    m_level->moveCursor(m_activeChar->entity->getPosition());
                }
            }
        }

        m_crt.asTarget()->draw(m_uiContext);
        target->draw(m_crt);
    }

    void OnlineGame::initializePlayerCharacters() {
        auto apollo = m_level->getApollo();
        sf::Packet packet;
        PlayerSpawnPayload payload;
        std::shared_ptr<Character> player;
        LocalPlayerTurn localPlayerTurn(&m_uiContext, m_lobby.sockets);
        if (m_lobby.isHost) {
            for (size_t id = 0; id < m_lobby.sockets.size() + 1; ++id) {
                payload.pos = sf::Vector2i{int(id), 0};
                payload.color = sf::Color(m_rand()); // TODO rand progression!
                payload.color.a = 255;
                for (size_t sockId = 0; sockId < m_lobby.sockets.size(); ++sockId) {
                    payload.local = sockId == id;
                    m_lobby.sockets[sockId]->send(PackagePayload(packet, payload));
                }
                player = std::make_shared<Character>();
                player->entity = std::make_shared<padi::LivingEntity>(
                        m_lobby.names[id],
                        apollo->lookupAnimContext("cube"),
                        payload.pos
                );
                player->entity->setColor(payload.color);

                player->abilities.push_back(std::make_shared<padi::content::Walk>(player->entity, 8));
                player->abilities.push_back(std::make_shared<padi::content::Teleport>(player->entity));
                player->abilities.push_back(std::make_shared<padi::content::Lighten>(player->entity));
                player->abilities.push_back(std::make_shared<padi::content::Dash>(player->entity, 8));
                player->abilities.push_back(std::make_shared<padi::content::Darken>(player->entity));
                if(m_lobby.sockets.size() > id) {
                    RemotePlayerTurn remotePlayerTurn(m_lobby.sockets[id]);
                    player->controller = [=](const std::shared_ptr<Level> &l,
                                             const std::shared_ptr<Character> &c) mutable {
                        return remotePlayerTurn(l, c);
                    };
                }

                auto spawnEvent = std::make_shared<padi::SpawnEvent>(player->entity);
                spawnEvent->dispatch(m_level);
                m_characters.push(player);
            }
            m_characters.back()->controller = [=](const std::shared_ptr<Level> &l,
                                                  const std::shared_ptr<Character> &c) mutable {
                return localPlayerTurn(l, c);
            };
        } else {
            auto & host = m_lobby.sockets.front();
            RemotePlayerTurn remotePlayerTurn(host);
            for (size_t id = 0; id < m_lobby.sockets.size() + 1; ++id) {
                auto status = host->receive(packet);
                if (status != sf::Socket::Done) {
                    printf("[OnlineGame|Client] Error occurred while receiving spawn event!\n");
                    exit(-1);
                }
                ReconstructPayload(packet, payload);
                player = std::make_shared<Character>();
                player->entity = std::make_shared<padi::LivingEntity>(
                        m_lobby.names[id],
                        apollo->lookupAnimContext("cube"),
                        payload.pos
                );
                player->entity->setColor(payload.color);
                player->abilities.push_back(std::make_shared<padi::content::Walk>(player->entity, 8));
                player->abilities.push_back(std::make_shared<padi::content::Teleport>(player->entity));
                player->abilities.push_back(std::make_shared<padi::content::Lighten>(player->entity));
                player->abilities.push_back(std::make_shared<padi::content::Dash>(player->entity, 8));
                player->abilities.push_back(std::make_shared<padi::content::Darken>(player->entity));
                if(payload.local) {
                    player->controller = [=](const std::shared_ptr<Level> &l,
                                                          const std::shared_ptr<Character> &c) mutable {
                        return localPlayerTurn(l, c);
                    };
                } else {
                    player->controller = [=](const std::shared_ptr<Level> &l,
                                                          const std::shared_ptr<Character> &c) mutable {
                        return remotePlayerTurn(l, c);
                    };
                }

                auto spawnEvent = std::make_shared<padi::SpawnEvent>(player->entity);
                spawnEvent->dispatch(m_level);
                m_characters.push(player);
            }
        }
        m_activeChar = m_characters.front();
        m_characters.pop();
    }

    void OnlineGame::propagateSeed() {
        sf::Packet packet;
        GameSeedPayload gameSeedPL;
        if (m_lobby.isHost) {
            gameSeedPL.seed = m_seed;
            PackagePayload(packet, gameSeedPL);
            printf("[OnlineGame|Server] Propagating seed!\n");
            for (auto &socket: m_lobby.sockets) {
                socket->send(packet);
            }
            printf("[OnlineGame|Server] Propagated seed %u!\n", m_seed);
        } else {
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
    }

    void OnlineGame::propagateLobby(std::string const &name) {
        sf::Packet packet;
        LobbySizePayload lobbySizePL;
        PlayerNamePayload namePL;
        if (m_lobby.isHost) {
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

                auto & playerName = m_lobby.names[id];
                namePL.player = id;
                std::memcpy(&namePL.name, playerName.c_str(), std::min(8ull, playerName.length()));
                PackagePayload(packet, namePL);

                for (size_t sockId = 0; sockId < m_lobby.sockets.size(); ++sockId) {
                    if (sockId != id) {
                        m_lobby.sockets[sockId]->send(packet);
                    }
                }
            }

            // propagate own name
            std::memcpy(&namePL.name, name.c_str(), std::min(8ull, name.length()));
            namePL.player = m_lobby.sockets.size();
            printf("[OnlineGame|Server] Propagating own name %i: %.*s!\n", namePL.player, 8, namePL.name);
            m_lobby.names.back() = name;
            PackagePayload(packet, namePL);
            for (auto & socket : m_lobby.sockets) {
                socket->send(packet);
            }
        } else {
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
            std::memcpy(&namePL.name, name.c_str(), std::min(8ull, name.length()));
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

    void OnlineGame::close() {
        for(auto & socket : m_lobby.sockets) {
            socket->disconnect();
            socket.reset();
        }
        while(!m_characters.empty()) m_characters.pop();
        m_activeChar.reset();
    }

} // content