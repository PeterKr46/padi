//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <queue>
#include <random>
#include <SFML/Network.hpp>

#include "../../ui/UIContext.h"
#include "../Activity.h"
#include "Character.h"
#include "../vfx/CRTMonitor.h"

namespace padi {
    class Level;
}

namespace padi::content {

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
