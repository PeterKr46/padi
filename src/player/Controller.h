//
// Created by Peter on 02/05/2022.
//

#pragma once

#include <memory>
#include "../entity/LivingEntity.h"

namespace padi {

    class Controller {
    public:
        explicit Controller(std::shared_ptr<LivingEntity> entity);

        virtual void startTurn(padi::Level *lvl) = 0;

        virtual bool turnComplete() = 0;

    protected:
        std::shared_ptr<LivingEntity> const m_entity;
    };

} // padi
