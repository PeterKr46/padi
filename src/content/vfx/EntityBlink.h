//
// Created by Peter on 08/07/2022.
//

#pragma once

#include "../../level/Level.h"
#include "../../level/CycleListener.h"
#include "../../entity/LivingEntity.h"

namespace padi::content {

    template<typename Colorable>
    class EntityBlink : public padi::CycleListener {
    private:
        std::weak_ptr<Colorable> m_target;
        int m_frequency = 4;

        sf::Color m_colors[2]{sf::Color(96, 0, 0, 255), sf::Color(96, 0, 0, 255)};
        int m_counter = 0;
    public:
        EntityBlink(std::weak_ptr<Colorable> livingEntity, int frequency)
                : m_target(std::move(livingEntity)),
                  m_frequency(frequency) {
            m_colors[0] = m_target.lock()->getColor();
        }

        bool onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) override {
            m_counter = (m_counter + 1) % m_frequency;
            if (!m_target.expired()) {
                if (!m_counter) {
                    std::swap(*m_colors, m_colors[1]);
                    m_target.lock()->setColor(m_colors[0]);
                }
                return true;
            }
            return false;
        }
    };

} // content
