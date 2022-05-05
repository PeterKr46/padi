//
// Created by Peter on 02/05/2022.
//

#pragma once

#include "LivingEntity.h"

namespace padi {

    class OneshotEntity : public padi::SlaveEntity, public CycleListener, public std::enable_shared_from_this<OneshotEntity> {
        bool onCycleEnd(Level *) override
    };

} // padi
