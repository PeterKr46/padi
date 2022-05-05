//
// Created by Peter on 28/04/2022.
//

#pragma once

#include <functional>

namespace padi {

    class DelayedAction {

        DelayedAction(std::function<void()>  action, size_t frames_before)
                : m_action(std::move(action)), m_framesBefore(frames_before) {

        }

        std::function<void()> m_action;
        size_t m_framesBefore{0};
    };

    // Returns true if to be repeated for next frame
    typedef std::function<bool(padi::Stage *)> FrameListener;

} // padi

