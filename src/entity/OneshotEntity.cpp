//
// Created by Peter on 02/05/2022.
//

#include "OneshotEntity.h"
#include "../media/AudioPlayback.h"

namespace padi {

    OneshotEntity::OneshotEntity(const sf::Vector2i &pos, uint32_t type) : StaticEntity(pos, type) {

    }

    bool OneshotEntity::onCycleBegin(std::weak_ptr<padi::Level> const &lvl) {
        auto level = lvl.lock();
        level->addCycleEndListener(shared_from_this());
        level->getMap()->addEntity(shared_from_this());
        return false;
    }

    bool OneshotEntity::onCycleEnd(std::weak_ptr<padi::Level> const &level) {
        level.lock()->getMap()->removeEntity(shared_from_this());
        return false;
    }

    void OneshotEntity::dispatch(Level * l) {
        l->addCycleBeginListener(shared_from_this());
    }

    OneshotEntityStack::OneshotEntityStack(const sf::Vector2i &pos, uint32_t type) : EntityStack(pos, type) {

    }

    bool OneshotEntityStack::onCycleBegin(std::weak_ptr<padi::Level> const &lvl) {
        auto level = lvl.lock();

        level->addCycleEndListener(shared_from_this());
        level->getMap()->addEntity(shared_from_this());
        return false;
    }

    bool OneshotEntityStack::onCycleEnd(std::weak_ptr<padi::Level> const &level) {

        level.lock()->getMap()->removeEntity(shared_from_this());
        return false;
    }

    void OneshotEntityStack::dispatch(Level * l) {
        l->addCycleBeginListener(shared_from_this());
    }
} // padi