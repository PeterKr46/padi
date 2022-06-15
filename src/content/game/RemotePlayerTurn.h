//
// Created by Peter on 15/06/2022.
//

#pragma once

#include <memory>

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
        bool operator()(const std::shared_ptr<Level> &level, const std::shared_ptr<Character> &character);
    };

} // content
