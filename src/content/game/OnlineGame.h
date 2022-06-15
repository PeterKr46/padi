//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <queue>
#include <random>

#include "../../ui/UIContext.h"
#include "../Activity.h"
#include "Character.h"
#include "CRTMonitor.h"
#include "SFML/Network/TcpSocket.hpp"

namespace padi {
    class Level;
}

namespace padi::content {
    struct alignas(64) LobbySizePayload {
        const uint8_t type = 6;
        uint8_t players{};
    };

    struct alignas(64) NamePayload {
        const uint8_t type = 7;
        uint8_t player{};
        char name[8] = "\0";
    };

    struct alignas(64) SeedPropagationPayload {
        const uint8_t type = 8;
        uint32_t seed{};
    };

    struct alignas(64) PlayerSpawnPayload {
        const uint8_t type = 9;
        sf::Vector2i pos;
        sf::Color color;
        bool local{};
    };

    class OnlineGame : public padi::Activity {
    public:
        OnlineGame(std::vector<std::shared_ptr<sf::TcpSocket>> sockets, bool hosting, std::string const &name,
                   uint32_t seed = 8008135);

        void handleResize(int width, int height) override;

        void draw(sf::RenderTarget *target) override;

        std::shared_ptr<padi::Activity> handoff() override;

    private:

        void initializePlayerCharacters();

        void propagateLobby(std::string const &name);

        void propagateSeed();

    private:
        padi::content::CRTMonitor m_crt;
        padi::UIContext m_uiContext;

        uint32_t m_seed;
        std::mt19937 m_rand;
        std::shared_ptr<Level> m_level;
        struct {
            bool isHost{false};
            std::vector<std::shared_ptr<sf::TcpSocket>> sockets;
            std::vector<std::string> names;
        } m_lobby;

        std::queue<std::shared_ptr<Character>> m_characters;
        std::shared_ptr<Character> m_activeChar;
    };

} // content
