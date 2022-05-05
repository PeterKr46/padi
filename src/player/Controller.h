//
// Created by Peter on 02/05/2022.
//

#pragma once

#include <memory>
#include "../entity/LivingEntity.h"

namespace padi {

    class Controller {

    protected:
        std::shared_ptr<LivingEntity> m_entity;
    };

} // padi
