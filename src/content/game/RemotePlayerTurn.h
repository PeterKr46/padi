//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <memory>
#include "../../net/Inbox.h"

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
        explicit RemotePlayerTurn(Inbox   socket);
        bool operator()(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &chr);
    private:
        int8_t m_activeAbility = -1;
        Inbox m_socket;
    };

} // content
