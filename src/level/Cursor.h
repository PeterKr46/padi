//
// Created by Peter on 01/05/2022.
//
#pragma once

#include "Level.h"
#include <SFML/Window/Keyboard.hpp>
#include "../entity/LivingEntity.h"

namespace padi {

    class Cursor
            : public padi::CycleListener, public padi::StaticEntity, public std::enable_shared_from_this<Cursor> {
    public:
        explicit Cursor(std::shared_ptr<padi::Animation> anim);

        void update(padi::Level *level);

    private:
        bool m_locked{false};
    };

} // padi
