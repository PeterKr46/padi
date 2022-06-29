//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <memory>
#include <vector>
#include "../../net/InOutBox.h"


namespace padi {

    class UIContext;

    namespace content {
        struct Character;
        class OnlineGame;
    }
}

namespace padi::content {

    class LocalPlayerTurn {
    public:
        explicit LocalPlayerTurn(UIContext *uiContext);

        bool operator()(const std::shared_ptr<OnlineGame> &game, const std::shared_ptr<Character> &chr);

    private:
        padi::UIContext *m_uiContext;
        int64_t m_activeAbility = -1;
        bool m_hasCast = false;
    private:
        enum LocalTurnState : int {
            START = -1,
            IDLE = 0,
            SELECTING = 1,
            CASTING = 2,
            DONE = 3,
        };
        void transitionUI(LocalTurnState from, LocalTurnState to, sf::Color emphColor = sf::Color::Yellow);
    };

} // content

