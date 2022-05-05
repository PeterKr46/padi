//
// Created by Peter on 28/04/2022.
//

#pragma once

#include <functional>

namespace padi {

    class DelayedAction {

        DelayedAction(std::function<void()>  action, size_t frames_before)
                : m_action(std::move(action)), m_before(frames_before) {

        }

        std::function<void()> m_action;
        size_t m_before{0};
    };

} // padi

