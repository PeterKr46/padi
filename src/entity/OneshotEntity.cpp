//
// Created by Peter on 02/05/2022.
//

#include "OneshotEntity.h"

namespace padi {

    OneshotEntity::OneshotEntity(const sf::Vector2i &pos) : SlaveEntity(pos) {

    }

    bool OneshotEntity::onCycleBegin(Level * level) {
        level->addCycleEndListener(shared_from_this());
        level->getMap()->addEntity(shared_from_this());
        return false;
    }

    bool OneshotEntity::onCycleEnd(Level * level) {
        level->getMap()->removeEntity(shared_from_this());
        return false;
    }

} // padi