//
// Created by Peter on 01/05/2022.
//

#include "SpawnEvent.h"

#include <utility>
#include "../map/Tile.h"

namespace padi {
    SpawnEvent::SpawnEvent(std::shared_ptr<LivingEntity> entity, std::shared_ptr<padi::Animation> particles)
            : m_entity(std::move(entity)) {
        m_particles = std::make_shared<StaticEntity>(m_entity->getPosition());
        m_particles->m_animation = std::move(particles);
    }

    SpawnEvent::SpawnEvent(std::shared_ptr<LivingEntity> entity, std::shared_ptr<padi::Animation> particles,
                           sf::Vector2i const& pos)    : m_entity(std::move(entity)) {
        m_particles = std::make_shared<StaticEntity>(pos);
        m_particles->m_animation = std::move(particles);
    }

    bool SpawnEvent::onCycleBegin(Level *level) {
        level->getMap()->addEntity(m_particles);
        level->addCycleEndListener(shared_from_this());
        level->addCycleEndListener(m_entity);
        level->addFrameBeginListener(shared_from_this());
        return false;
    }

    bool SpawnEvent::onCycleEnd(Level *level) {
        level->getMap()->removeEntity(m_particles);

        level->addCycleBeginListener(m_entity);
        return false;
    }

    void SpawnEvent::dispatch(std::shared_ptr<Level> const& level) {
        level->addCycleBeginListener(shared_from_this());
    }

    bool SpawnEvent::onFrameBegin(Level *level, uint8_t frame) {
        if(pullFocus) level->centerView(m_particles->getPosition());
        static const int radius = 8;
        auto pos = m_particles->getPosition();
        for (int x = -radius; x < radius; ++x)
            for (int y = -radius; y < radius; ++y) {
                auto tile = level->getMap()->getTile(pos.x + x, pos.y + y);
                float power = 32 * (1 - std::min<float>(1.f, std::sqrt(x*x + y*y)/radius));
                if (tile) {
                    tile->setColor(tile->getColor() + m_entity->getColor() * sf::Color(power, power, power));
                }
            }

        if (frame == 7) {
            level->getMap()->addEntity(m_entity, m_particles->getPosition(), ~0u);
            return false;
        }
        return true;
    }
}