//
// Created by Peter on 01/05/2022.
//

#pragma once

#include "Level.h"
#include "../entity/LivingEntity.h"
#include "../entity/EntityStack.h"
#include "../entity/OneshotEntity.h"

namespace padi {
    class SpawnEvent
            : public padi::CycleListener, public std::enable_shared_from_this<SpawnEvent> {
    public:
        explicit SpawnEvent(const std::shared_ptr<LivingEntity>& entity);
        explicit SpawnEvent(std::shared_ptr<LivingEntity> entity, sf::Vector2i const& pos);

        ~SpawnEvent() = default;

        void dispatch(std::shared_ptr<Level> const &level);

        bool onCycleBegin(Level *) override;

        bool onCycleEnd(Level *) override;

        bool onFrameBegin(Level *, uint8_t frame) override;

        bool pullFocus{false};

    private:
        std::shared_ptr<OneshotEntityStack> m_ray;
        std::shared_ptr<LivingEntity> m_entity;
    };
}