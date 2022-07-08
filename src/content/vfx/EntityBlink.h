//
// Created by Peter on 08/07/2022.
//

#pragma once

#include "../../level/Level.h"
#include "../../level/CycleListener.h"
#include "../../entity/LivingEntity.h"

namespace padi::content {

    class EntityBlink : public padi::CycleListener {
    public:
        EntityBlink(std::weak_ptr<LivingEntity> livingEntity, int frequency);

        bool onFrameBegin(const std::weak_ptr<padi::Level> &lvl, uint8_t frame) override;

    private:
        std::weak_ptr<LivingEntity> m_target;
        int m_frequency = 4;

        sf::Color m_colors[2]{sf::Color(96,0,0, 255),sf::Color(96,0,0, 255)};
        int m_counter = 0;
    };

} // content
