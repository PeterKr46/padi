//
// Created by Peter on 01/05/2022.
//

#pragma once

#include "Level.h"
#include "../entity/LivingEntity.h"

namespace padi {
    class SpawnEvent
            : public padi::CycleListener
            , public std::enable_shared_from_this<SpawnEvent> {
    public:
        explicit SpawnEvent(std::shared_ptr<LivingEntity> entity, std::shared_ptr<padi::Animation> particles);
        ~SpawnEvent() = default;

        void dispatch(Level* level);

        bool onCycleBegin(Level *) override;
        bool onCycleEnd(Level *) override;
        bool onFrameBegin(Level *, uint8_t frame) override;

    private:
        std::shared_ptr<SlaveEntity>    m_particles;
        std::shared_ptr<LivingEntity>         m_entity;
    };
}