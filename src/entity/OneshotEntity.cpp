//
// Created by Peter on 02/05/2022.
//

#include "OneshotEntity.h"
#include "../media/AudioPlayback.h"

namespace padi {

    OneshotEntity::OneshotEntity(const sf::Vector2i &pos) : StaticEntity(pos) {

    }

    bool OneshotEntity::onCycleBegin(Level *level) {
        level->addCycleEndListener(shared_from_this());
        level->getMap()->addEntity(shared_from_this());
        return false;
    }

    bool OneshotEntity::onCycleEnd(Level *level) {
        level->getMap()->removeEntity(shared_from_this());
        return false;
    }

    OneshotEntityStack::OneshotEntityStack(const sf::Vector2i &pos) : EntityStack(pos) {

    }

    bool OneshotEntityStack::onCycleBegin(Level *level) {

        level->addCycleEndListener(shared_from_this());
        level->getMap()->addEntity(shared_from_this());
        return false;
    }

    bool OneshotEntityStack::onCycleEnd(Level *level) {

        level->getMap()->removeEntity(shared_from_this());
        return false;
    }
} // padi