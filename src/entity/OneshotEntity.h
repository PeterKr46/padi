//
// Created by Peter on 02/05/2022.
//

#pragma once

#include "StaticEntity.h"
#include "EntityStack.h"
#include "../level/Level.h"
#include <SFML/Audio/Sound.hpp>
#include "../Constants.h"
#include "../level/CycleListener.h"

namespace padi {

    /**
     * Single-cycle Entity that aims to self-destruct afterwards.
     *
     * Instantiate at chosen location and add CycleBegin listener
     */
    class OneshotEntity
            : public padi::StaticEntity, public CycleListener, public std::enable_shared_from_this<OneshotEntity> {
    public:
        explicit OneshotEntity(sf::Vector2i const &pos);

        bool onCycleBegin(Level *) override;

        bool onCycleEnd(Level *) override;
    };

    /**
     * Single-cycle Entity that aims to self-destruct afterwards.
     *
     * Instantiate at chosen location and add CycleBegin listener
     */
    class OneshotEntityStack
            : public padi::EntityStack, public CycleListener, public std::enable_shared_from_this<OneshotEntityStack> {
    public:
        explicit OneshotEntityStack(sf::Vector2i const &pos);

        bool onCycleBegin(Level *) override;

        bool onCycleEnd(Level *) override;
    };

} // padi
