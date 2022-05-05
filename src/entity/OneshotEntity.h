//
// Created by Peter on 02/05/2022.
//

#pragma once

#include "LivingEntity.h"

namespace padi {

    /**
     * Single-cycle Entity that aims to self-destruct afterwards.
     *
     * Instantiate at chosen location and add CycleBegin listener
     */
    class OneshotEntity : public padi::SlaveEntity, public CycleListener, public std::enable_shared_from_this<OneshotEntity> {
    public:
        explicit OneshotEntity(sf::Vector2i const& pos);
        bool onCycleBegin(Level *) override;
        bool onCycleEnd(Level *) override;
    };

} // padi
