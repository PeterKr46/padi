//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <memory>
#include "SFML/Network/TcpSocket.hpp"

namespace padi {

    class Level;

    class UIContext;

    namespace content {
        struct Character;
    }
}

namespace padi::content {

    class RemotePlayerTurn {
    public:
        explicit RemotePlayerTurn(std::shared_ptr<sf::TcpSocket>  socket);
        bool operator()(const std::shared_ptr<Level> &level, const std::shared_ptr<Character> &chr);
    private:
        bool m_turnStarted = false;
        uint8_t m_casting = 0;
        std::shared_ptr<sf::TcpSocket> m_socket;
    };

} // content
