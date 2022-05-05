//
// Created by Peter on 02/05/2022.
//

#pragma once

#include <array>
#include "../entity/LivingEntity.h"
#include "Ability.h"

namespace padi {

    class Player : public LivingEntity {
    public:


    protected:
        std::array<std::shared_ptr<padi::Ability>, 4> m_abilities;
    };

} // padi
