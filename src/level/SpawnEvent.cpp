//
// Created by Peter on 01/05/2022.
//

#include "SpawnEvent.h"

#include <utility>

namespace padi {
    SpawnEvent::SpawnEvent(std::shared_ptr<LivingEntity> entity, std::shared_ptr<padi::Animation> particles)
     : m_entity(std::move(entity)) {
        m_particles = std::make_shared<SlaveEntity>(m_entity->getPosition());
        m_particles->m_animation = std::move(particles);
    }

    bool SpawnEvent::onCycleBegin(Level * level) {
        level->getMap()->addEntity(m_particles, m_entity->getPosition());
        level->addCycleEndListener(shared_from_this());
        level->addCycleEndListener(m_entity);
        level->addFrameBeginListener(shared_from_this());
        return false;
    }

    bool SpawnEvent::onCycleEnd(Level * level) {
        level->getMap()->removeEntity(m_particles);

        level->addCycleBeginListener(m_entity);
        return false;
    }

    void SpawnEvent::dispatch(Level *level) {
        level->addCycleBeginListener(shared_from_this());
    }

    bool SpawnEvent::onFrameBegin(Level * level, uint8_t frame) {
        level->centerView(m_particles->getPosition());
        if(frame == 7) {
            level->getMap()->addEntity(m_entity, m_particles->getPosition(), ~0u);
            return false;
        }
        return true;
    }
}