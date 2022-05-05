//
// Created by Peter on 02/05/2022.
//

#include "OneshotEntity.h"

namespace padi {
    bool OneshotEntity::onCycleEnd(Level * level) {
        level->getMap()->removeEntity(shared_from_this());
        return false;
    }
} // padi