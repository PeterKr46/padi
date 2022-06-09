//
// Created by Peter on 02/05/2022.
//

#include "Controller.h"

#include <utility>

namespace padi {
    Controller::Controller(std::shared_ptr<LivingEntity> entity)
            : m_entity(std::move(entity)) {

    }
} // padi