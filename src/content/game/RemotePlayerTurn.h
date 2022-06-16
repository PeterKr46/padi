//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <memory>

namespace sf {
    class TcpSocket;
}

namespace padi {


    class UIContext;

    namespace content {
        struct Character;
        class OnlineGame;
    }
}

namespace padi::content {

    class RemotePlayerTurn {
    public:
        explicit RemotePlayerTurn(std::shared_ptr<sf::TcpSocket>  socket);
        bool operator()(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &chr);
    private:
        int8_t m_activeAbility = -1;
        std::shared_ptr<sf::TcpSocket> m_socket;
    };

} // content
