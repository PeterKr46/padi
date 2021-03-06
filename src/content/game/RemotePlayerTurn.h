//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <memory>
#include "../../net/InOutBox.h"

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
        explicit RemotePlayerTurn(InOutBox socket, bool reflect = false, uint32_t ignore = 0);
        bool operator()(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &chr);
    private:
        int8_t      m_activeAbility = -1;
        InOutBox    m_socket;
        bool        m_reflect{false};
        uint32_t    m_ignore;
    };

} // content
