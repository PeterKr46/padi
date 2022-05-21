//
// Created by Peter on 01/05/2022.
//

#include "SpawnEvent.h"

#include <utility>
#include <cmath>
#include "../map/Tile.h"
#include "../Constants.h"
#include "../entity/OneshotEntity.h"

namespace padi {

    SpawnEvent::SpawnEvent(const std::shared_ptr<LivingEntity>& entity)
            : SpawnEvent(entity, entity->getPosition()) {}
            /*m_entity(std::move(entity)) {
        m_entity->trySetAnimation("spawn");
        auto apolloCtx = m_entity->getAnimationSet();
        if (apolloCtx->find("spawn_ray") != apolloCtx->end()) {
            m_ray = std::make_shared<OneshotEntityStack>(m_entity->getPosition());
            m_ray->m_animation = apolloCtx->at("spawn_ray");
            m_ray->m_verticalOffset = padi::TileSize.y;
        }
    }*/

    SpawnEvent::SpawnEvent(std::shared_ptr<LivingEntity> entity, sf::Vector2i const &pos)
            : m_entity(std::move(entity)) {
        m_entity->trySetAnimation("spawn");
        auto apolloCtx = m_entity->getAnimationSet();
        if (apolloCtx->find("spawn_ray") != apolloCtx->end()) {
            m_ray = std::make_shared<OneshotEntityStack>(pos);
            m_ray->m_color = m_entity->getColor();
            m_ray->m_animation = apolloCtx->at("spawn_ray");
            m_ray->m_stackSize = 16;
            m_ray->m_verticalOffset = -padi::TileSize.y;
        }
    }

    bool SpawnEvent::onCycleBegin(Level *level) {
        if (m_ray) {
            level->getMap()->addEntity(m_entity, m_ray->getPosition(), ~0u);
            level->getMap()->addEntity(m_ray);
            level->addCycleEndListener(m_ray);
        }

        level->addCycleEndListener(shared_from_this());
        level->addFrameBeginListener(shared_from_this());
        level->addCycleEndListener(m_entity);
        return false;
    }

    bool SpawnEvent::onCycleEnd(Level *level) {
        level->addCycleBeginListener(m_entity);
        return false;
    }

    void SpawnEvent::dispatch(std::shared_ptr<Level> const &level) {
        level->addCycleBeginListener(shared_from_this());
    }

    bool SpawnEvent::onFrameBegin(Level *level, uint8_t frame) {
        sf::Vector2i pos = m_ray->getPosition();
        if (pullFocus)
            level->centerView(pos);

        static const int radius = 4;
        for (int x = -radius; x < radius; ++x)
            for (int y = -radius; y < radius; ++y) {
                auto tile = level->getMap()->getTile(pos.x + x, pos.y + y);
                float power = 32 * (1 - std::min<float>(1.f, std::sqrt(x * x + y * y) / radius));
                if (tile) {
                    tile->setColor(tile->getColor() + m_entity->getColor() * sf::Color(power, power, power));
                }
            }

        if (frame == 7) {
            return false;
        }
        return true;
    }
}