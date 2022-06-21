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
        explicit OneshotEntity(sf::Vector2i const &pos, uint32_t type = 0);

        bool onCycleBegin(std::weak_ptr<padi::Level> const &lvl) override;

        bool onCycleEnd(std::weak_ptr<padi::Level> const &lvl) override;

        void dispatch(std::weak_ptr<Level> const &level);
        void dispatchImmediate(std::weak_ptr<Level> const &level);
    };

    /**
     * Single-cycle Entity that aims to self-destruct afterwards.
     *
     * Instantiate at chosen location and add CycleBegin listener
     */
    class OneshotEntityStack
            : public padi::EntityStack, public CycleListener, public std::enable_shared_from_this<OneshotEntityStack> {
    public:
        explicit OneshotEntityStack(sf::Vector2i const &pos, uint32_t type = 0);

        bool onCycleBegin(std::weak_ptr<padi::Level> const &lvl) override;

        bool onCycleEnd(std::weak_ptr<padi::Level> const &lvl) override;

        void dispatch(std::weak_ptr<Level> const &level);
        void dispatchImmediate(std::weak_ptr<Level> const &level);
    };

} // padi
