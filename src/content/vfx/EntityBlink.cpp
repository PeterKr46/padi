//
// Created by Peter on 08/07/2022.
//

#include "EntityBlink.h"

#include <utility>

namespace padi::content {

   /* template<typename Colorable>
    bool EntityBlink<Colorable>::onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) {
        m_counter = (m_counter + 1) % m_frequency;
        if(!m_target.expired()) {
            if (!m_counter) {
                std::swap(*m_colors, m_colors[1]);
                m_target.lock()->setColor(m_colors[0]);
            }
            return true;
        }
        return false;
    }

    template<typename Colorable>
    EntityBlink<Colorable>::EntityBlink(std::weak_ptr<Colorable> livingEntity, int frequency)
            : m_target(std::move(livingEntity)), m_frequency(frequency) {
        m_colors[0] = m_target.lock()->getColor();
    }*/
} // content