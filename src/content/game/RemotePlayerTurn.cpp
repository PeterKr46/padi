//
// Created by Peter on 15/06/2022.
//

#include "RemotePlayerTurn.h"

namespace padi::content {
        bool RemotePlayerTurn::operator()(const std::shared_ptr<Level> &level, const std::shared_ptr<Character> &character) {
            return false;
        }
    } // content