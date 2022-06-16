//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <memory>
#include <vector>

namespace sf {
    class TcpSocket;
}

namespace padi {

    class Level;

    class UIContext;

    namespace content {
        struct Character;
    }
}

namespace padi::content {

    class LocalPlayerTurn {
    public:
        explicit LocalPlayerTurn(UIContext *uiContext, std::vector<std::shared_ptr<sf::TcpSocket>> & sockets);

        bool operator()(const std::shared_ptr<Level> &level, const std::shared_ptr<Character> &character);

    private:
        padi::UIContext *m_uiContext;
        std::vector<std::shared_ptr<sf::TcpSocket>> m_sockets;
        int8_t m_activeAbility = -1;
        bool m_hasCast = false;
    };

} // content

